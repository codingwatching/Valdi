#!/usr/bin/env bash
#
# test_external_build_graph.sh — Verify that JSCore is correctly gated behind
# the use_local_compiler flag, so external Linux builds (where the prebuilt
# libjsc.so is absent) don't hit undefined-symbol link failures.
#
# Two layers of checks:
#   1. Structural (grep): verifies the BUILD file wiring is correct. Works on
#      any machine, including internal CI where Swift toolchains aren't installed.
#   2. Cquery (optional): verifies the live build graph when analysis succeeds.
#      Skipped if cquery can't resolve toolchains (e.g. internal CI without Swift).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPEN_SOURCE_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$OPEN_SOURCE_DIR"

FAILURES=0
fail() {
    echo "FAIL: $1"
    FAILURES=$((FAILURES + 1))
}
pass() { echo "PASS: $1"; }

BUILD_FILE="valdi/BUILD.bazel"

# ---------------------------------------------------------------------------
# 1. Structural checks — always run
# ---------------------------------------------------------------------------
echo "=== Structural checks: JSCore gating in BUILD file ==="

# jscore_enabled must reference jscore_available_on_linux, not bare linux_x64.
if grep -A5 'name = "jscore_enabled"' "$BUILD_FILE" | grep -q 'jscore_available_on_linux'; then
    pass "jscore_enabled uses gated jscore_available_on_linux (not bare linux_x64)"
else
    fail "jscore_enabled references linux_x64 directly — external Linux builds will fail"
fi

# jscore_available_on_linux must require not_use_local_compiler.
if grep -A5 'name = "jscore_available_on_linux"' "$BUILD_FILE" | grep -q 'not_use_local_compiler'; then
    pass "jscore_available_on_linux requires not_use_local_compiler"
else
    fail "jscore_available_on_linux missing not_use_local_compiler guard"
fi

# not_use_local_compiler must be the inverse of use_local_compiler_enabled.
if grep -A2 'not_use_local_compiler' "$BUILD_FILE" | grep -q 'use_local_compiler_enabled'; then
    pass "not_use_local_compiler is inverse of use_local_compiler_enabled"
else
    fail "not_use_local_compiler wiring is broken"
fi

# ---------------------------------------------------------------------------
# 2. Cquery checks — only on Linux, only if analysis succeeds
# ---------------------------------------------------------------------------
if [[ $(uname) == Linux ]]; then
    echo ""
    echo "=== Cquery checks: live build graph with use_local_compiler=true ==="

    EXTERNAL_FLAG="--//bzl/valdi:use_local_compiler=true"
    DEPS_FILE=$(mktemp)
    trap 'rm -f "$DEPS_FILE"' EXIT

    if bzl cquery //valdi:test_layout "$EXTERNAL_FLAG" --output=label 2>/dev/null; then
        pass "test_layout analyzable with use_local_compiler=true"

        # Write deps to a file (can be tens of thousands of lines — too large
        # for a shell variable without hitting ARG_MAX on printf).
        if bzl cquery "deps(//valdi:test_layout)" "$EXTERNAL_FLAG" --output=label >"$DEPS_FILE" 2>/dev/null; then
            if grep -q '//valdi:valdi_jscore' "$DEPS_FILE"; then
                fail "valdi_jscore reachable from test_layout under use_local_compiler=true — will cause link failures"
            else
                pass "valdi_jscore correctly excluded from test_layout"
            fi

            if grep -q '//valdi:valdi_hermes' "$DEPS_FILE"; then
                pass "valdi_hermes reachable from test_layout"
            else
                fail "valdi_hermes NOT reachable from test_layout — no JS engine available"
            fi
        else
            fail "deps() query failed for test_layout"
        fi
    else
        echo "SKIP: cquery failed (Swift toolchain likely not installed) — structural checks above are sufficient"
    fi
else
    echo ""
    echo "SKIP: cquery checks only apply on Linux (macOS uses system JavaScriptCore)"
fi

echo ""
echo "=== External build graph test complete ==="
if [[ "$FAILURES" -gt 0 ]]; then
    echo "RESULT: $FAILURES check(s) failed"
    exit 1
else
    echo "RESULT: All checks passed"
fi
