#!/usr/bin/env bash
# Bazel credential helper for the GCS remote cache used by CI.
#
# Bazel invokes this on every remote-cache request (subject to its `expires`
# caching), so a single long-running `bazel build` can rotate auth tokens
# mid-build instead of being stuck with the bearer set at startup.
#
# Spec: https://github.com/EngFlow/credential-helper-spec
#   Invocation: `<helper> get` with `{"uri": "https://..."}` on stdin
#   Response:   `{"headers": {"Authorization": ["Bearer ..."]}, "expires": "..."}`
#
# Implements the GitHub-OIDC → Google-STS → optional SA-impersonation chain
# directly with curl + jq, so we don't depend on `gcloud` being installed on
# the runner (notably, macos-latest GitHub-hosted runners no longer ship the
# Google Cloud SDK preinstalled, and `google-github-actions/auth@v2` only
# writes the WIF credential file — it doesn't install gcloud).
#
# Performance: Bazel may invoke this helper many times concurrently at the
# start of a build. To stay under Bazel's per-helper timeout we cache the
# minted token + expires on disk and serialize the cold mint with flock so
# only one process ever does the 3-hop HTTP chain.

set -euo pipefail

LOG_FILE="${BAZEL_GCS_CRED_HELPER_LOG:-/tmp/bazel-cred-helper.log}"
CACHE_FILE="${BAZEL_GCS_CRED_HELPER_CACHE:-/tmp/bazel-cred-helper-token.json}"

# How long (seconds) before the underlying token's expireTime we treat the
# cache as stale and re-mint. Gives Bazel a buffer to retry without ever
# sending a token that's about to expire.
REFRESH_LEAD_SECONDS=120

log() {
  printf '[%s] pid=%d %s\n' "$(date -u +'%Y-%m-%dT%H:%M:%SZ')" "$$" "$*" \
    >> "$LOG_FILE" 2>/dev/null || true
}

emit_empty() {
  printf '{"headers": {}}\n'
}

if [ "${1:-}" != "get" ]; then
  exit 0
fi

# Drain stdin — we only ever handle storage.googleapis.com so the URI is
# informational, but Bazel will hang if we don't consume it.
cat >/dev/null

CREDS_FILE="${GOOGLE_APPLICATION_CREDENTIALS:-}"
if [ -z "$CREDS_FILE" ] || [ ! -f "$CREDS_FILE" ]; then
  # Fork-PR / no-creds path: emit empty headers so Bazel falls back to
  # building without remote cache, same as the pre-auth behaviour.
  log "no GOOGLE_APPLICATION_CREDENTIALS — emitting empty headers"
  emit_empty
  exit 0
fi

for tool in jq curl date; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    log "FATAL: $tool not on PATH (PATH=$PATH)"
    exit 1
  fi
done

# Try the cache first without taking the lock — saves the slow path on the
# 99% case where the token is still fresh.
emit_cached_if_fresh() {
  [ -f "$CACHE_FILE" ] || return 1
  local cached expires expires_normalized expires_epoch now_epoch threshold_epoch
  cached="$(cat "$CACHE_FILE" 2>/dev/null)" || return 1
  expires="$(printf '%s' "$cached" | jq -r '.expires // empty')" || return 1
  [ -n "$expires" ] || return 1
  # Google's generateAccessToken returns expireTime with fractional seconds
  # (e.g. 2024-03-12T15:01:23.045Z). macOS `date -j -f` strict-parses the
  # format string, so strip the fractional component before parsing.
  expires_normalized="$(printf '%s' "$expires" | sed 's/\.[0-9]*Z$/Z/')"
  expires_epoch="$(date -j -f '%Y-%m-%dT%H:%M:%SZ' "$expires_normalized" +%s 2>/dev/null \
    || date -u -d "$expires_normalized" +%s 2>/dev/null)" || return 1
  now_epoch="$(date -u +%s)"
  threshold_epoch=$((expires_epoch - REFRESH_LEAD_SECONDS))
  if [ "$now_epoch" -lt "$threshold_epoch" ]; then
    printf '%s\n' "$cached"
    return 0
  fi
  return 1
}

if emit_cached_if_fresh; then
  log "cache hit (fast path)"
  exit 0
fi

# Slow path: serialize mints across concurrent helper processes so we only
# do the HTTP chain once per refresh window. `mkdir` is atomic on POSIX and
# avoids depending on flock(1) (not installed by default on macOS).
#
# We write our PID into the lockdir on acquire; waiters use `kill -0` to
# detect a dead holder and reclaim immediately. This avoids a time-based
# grace period that can deadlock under Bazel's --credential_helper_timeout
# (Bazel SIGKILLs the holder before any reasonable grace elapses).
LOCK_DIR="${CACHE_FILE}.lockdir"
LOCK_DEADLINE=$(( $(date -u +%s) + 60 ))
while ! mkdir "$LOCK_DIR" 2>/dev/null; do
  if [ "$(date -u +%s)" -ge "$LOCK_DEADLINE" ]; then
    log "FATAL: could not acquire lock on $LOCK_DIR within 60s"
    exit 1
  fi
  # If the lock-holder is no longer running, reclaim immediately.
  if [ -f "$LOCK_DIR/pid" ]; then
    holder_pid="$(cat "$LOCK_DIR/pid" 2>/dev/null || true)"
    if [ -n "$holder_pid" ] && ! kill -0 "$holder_pid" 2>/dev/null; then
      log "stale lock on $LOCK_DIR (holder pid=$holder_pid is gone) — removing"
      rm -rf "$LOCK_DIR" 2>/dev/null || true
      continue
    fi
  fi
  sleep 0.2
done
trap 'rm -rf "$LOCK_DIR" 2>/dev/null || true' EXIT
printf '%s\n' "$$" > "$LOCK_DIR/pid"

# Re-check the cache under the lock — another process may have refreshed
# while we were waiting.
if emit_cached_if_fresh; then
  log "cache hit (post-lock)"
  exit 0
fi

OIDC_TOKEN="${ACTIONS_ID_TOKEN_REQUEST_TOKEN:-}"
OIDC_URL="${ACTIONS_ID_TOKEN_REQUEST_URL:-}"
if [ -z "$OIDC_TOKEN" ] || [ -z "$OIDC_URL" ]; then
  log "FATAL: ACTIONS_ID_TOKEN_REQUEST_{TOKEN,URL} unset; cannot mint OIDC subject token"
  exit 1
fi

AUDIENCE="$(jq -r '.audience // empty' "$CREDS_FILE")"
TOKEN_URL="$(jq -r '.token_url // empty' "$CREDS_FILE")"
SUBJECT_TOKEN_TYPE="$(jq -r '.subject_token_type // empty' "$CREDS_FILE")"
IMPERSONATION_URL="$(jq -r '.service_account_impersonation_url // empty' "$CREDS_FILE")"

if [ -z "$AUDIENCE" ] || [ -z "$TOKEN_URL" ] || [ -z "$SUBJECT_TOKEN_TYPE" ]; then
  log "FATAL: credentials file at $CREDS_FILE missing required external_account fields"
  exit 1
fi

# Shared curl options — short timeouts + retry on transient errors so a flaky
# network blip doesn't blow past Bazel's --credential_helper_timeout.
CURL_OPTS=(
  --silent --show-error --fail-with-body
  --connect-timeout 5 --max-time 15
  --retry 3 --retry-delay 1 --retry-all-errors
)

ENCODED_AUDIENCE="$(printf '%s' "$AUDIENCE" | jq -sRr @uri)"
OIDC_RESPONSE="$(curl "${CURL_OPTS[@]}" \
  -H "Authorization: Bearer $OIDC_TOKEN" \
  -H "Accept: application/json; api-version=2.0" \
  "${OIDC_URL}&audience=${ENCODED_AUDIENCE}" 2>>"$LOG_FILE")" || {
    log "FATAL: GitHub OIDC fetch failed (curl exit $?): $OIDC_RESPONSE"
    exit 1
  }

SUBJECT_TOKEN="$(printf '%s' "$OIDC_RESPONSE" | jq -r '.value // empty')"
if [ -z "$SUBJECT_TOKEN" ]; then
  log "FATAL: GitHub OIDC response missing .value: $OIDC_RESPONSE"
  exit 1
fi
log "minted GitHub OIDC subject token (len=${#SUBJECT_TOKEN})"

STS_RESPONSE="$(curl "${CURL_OPTS[@]}" -X POST "$TOKEN_URL" \
  -H "Content-Type: application/x-www-form-urlencoded" \
  --data-urlencode "audience=$AUDIENCE" \
  --data-urlencode "grant_type=urn:ietf:params:oauth:grant-type:token-exchange" \
  --data-urlencode "requested_token_type=urn:ietf:params:oauth:token-type:access_token" \
  --data-urlencode "scope=https://www.googleapis.com/auth/cloud-platform" \
  --data-urlencode "subject_token_type=$SUBJECT_TOKEN_TYPE" \
  --data-urlencode "subject_token=$SUBJECT_TOKEN" 2>>"$LOG_FILE")" || {
    log "FATAL: Google STS exchange failed (curl exit $?): $STS_RESPONSE"
    exit 1
  }

FEDERATED_TOKEN="$(printf '%s' "$STS_RESPONSE" | jq -r '.access_token // empty')"
if [ -z "$FEDERATED_TOKEN" ]; then
  log "FATAL: Google STS response missing access_token: $STS_RESPONSE"
  exit 1
fi

if [ -n "$IMPERSONATION_URL" ]; then
  IMP_RESPONSE="$(curl "${CURL_OPTS[@]}" -X POST "$IMPERSONATION_URL" \
    -H "Authorization: Bearer $FEDERATED_TOKEN" \
    -H "Content-Type: application/json" \
    --data '{"scope":["https://www.googleapis.com/auth/cloud-platform"]}' 2>>"$LOG_FILE")" || {
      log "FATAL: SA impersonation failed (curl exit $?): $IMP_RESPONSE"
      exit 1
    }

  ACCESS_TOKEN="$(printf '%s' "$IMP_RESPONSE" | jq -r '.accessToken // empty')"
  EXPIRES="$(printf '%s' "$IMP_RESPONSE" | jq -r '.expireTime // empty')"
  if [ -z "$ACCESS_TOKEN" ] || [ -z "$EXPIRES" ]; then
    log "FATAL: SA impersonation response missing accessToken/expireTime: $IMP_RESPONSE"
    exit 1
  fi
  log "minted impersonated access token, expires=$EXPIRES"
else
  ACCESS_TOKEN="$FEDERATED_TOKEN"
  EXPIRES_IN="$(printf '%s' "$STS_RESPONSE" | jq -r '.expires_in // 3600')"
  EXPIRES="$(date -u -v+"${EXPIRES_IN}"S +'%Y-%m-%dT%H:%M:%SZ' 2>/dev/null \
    || date -u -d "+${EXPIRES_IN} seconds" +'%Y-%m-%dT%H:%M:%SZ')"
  log "minted federated access token (no SA impersonation), expires=$EXPIRES"
fi

RESPONSE_JSON="$(jq -nc \
  --arg token "$ACCESS_TOKEN" \
  --arg expires "$EXPIRES" \
  '{headers: {Authorization: ["Bearer " + $token]}, expires: $expires}')"

# Atomic cache write so partial files never get read by a concurrent process.
TMP_CACHE="$(mktemp "${CACHE_FILE}.XXXXXX")"
printf '%s\n' "$RESPONSE_JSON" > "$TMP_CACHE"
mv -f "$TMP_CACHE" "$CACHE_FILE"

printf '%s\n' "$RESPONSE_JSON"
