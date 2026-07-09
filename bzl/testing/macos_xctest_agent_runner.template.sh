#!/bin/bash

set -euo pipefail

# Runs a macOS `.xctest` bundle via the `xctest` agent. The `%(key)s` values
# are substituted by the macos_unit_test rule before this script executes; see
# macos_xctest_agent_runner.bzl for why this avoids xcodebuild/testmanagerd.

if [[ -n "${TEST_PREMATURE_EXIT_FILE:-}" ]]; then
  touch "$TEST_PREMATURE_EXIT_FILE"
fi

if [[ "%(test_type)s" == "XCUITEST" ]]; then
  echo "This runner only supports macos_unit_test." >&2
  exit 1
fi

basename_without_extension() {
  local filename
  filename=$(basename "$1")
  echo "${filename%.*}"
}

TEST_TMP_DIR="$(mktemp -d "${TEST_TMPDIR:-${TMPDIR:-/tmp}}/test_tmp_dir.XXXXXX")"
trap 'rm -rf "${TEST_TMP_DIR}"' ERR EXIT

TEST_BUNDLE_PATH="%(test_bundle_path)s"
TEST_BUNDLE_NAME=$(basename_without_extension "$TEST_BUNDLE_PATH")

if [[ "$TEST_BUNDLE_PATH" == *.xctest ]]; then
  cp -R "$TEST_BUNDLE_PATH" "$TEST_TMP_DIR"
  # Bazel marks outputs read-only; the agent needs a writable copy.
  chmod -R 777 "$TEST_TMP_DIR/$TEST_BUNDLE_NAME.xctest"
else
  unzip -qq -d "${TEST_TMP_DIR}" "${TEST_BUNDLE_PATH}"
fi
readonly test_bundle="$TEST_TMP_DIR/${TEST_BUNDLE_NAME}.xctest"
readonly test_binary="$test_bundle/Contents/MacOS/$TEST_BUNDLE_NAME"

# Propagate the rule's test environment, plus coverage output when requested.
TEST_ENV="%(test_env)s"
readonly profraw="$TEST_TMP_DIR/coverage.profraw"
if [[ "${COVERAGE:-}" -eq 1 ]]; then
  if [[ -n "$TEST_ENV" ]]; then
    TEST_ENV="$TEST_ENV,LLVM_PROFILE_FILE=$profraw"
  else
    TEST_ENV="LLVM_PROFILE_FILE=$profraw"
  fi
fi
if [[ -n "$TEST_ENV" ]]; then
  for single_env in ${TEST_ENV//,/ }; do
    export "$single_env"
  done
fi

# Bazel's --test_filter arrives as TESTBRIDGE_TEST_ONLY; the agent takes it via
# -XCTest. "All" runs every test in the bundle.
readonly xctest_filter="${TESTBRIDGE_TEST_ONLY:-All}"

readonly testlog="$TEST_TMP_DIR/test.log"
test_exit_code=0
xcrun xctest -XCTest "$xctest_filter" "$test_bundle" 2>&1 | tee -i "$testlog" \
  || test_exit_code=$?

if [[ "$test_exit_code" -ne 0 ]]; then
  echo "error: tests exited with '$test_exit_code'" >&2
  exit "$test_exit_code"
fi

if [[ "${COVERAGE:-}" -ne 1 ]]; then
  if [[ -f "${TEST_PREMATURE_EXIT_FILE:-}" ]]; then
    rm -f "$TEST_PREMATURE_EXIT_FILE"
  fi
  exit 0
fi

llvm_coverage_manifest="$COVERAGE_MANIFEST"
readonly provided_coverage_manifest="%(test_coverage_manifest)s"
if [[ -s "${provided_coverage_manifest:-}" ]]; then
  llvm_coverage_manifest="$provided_coverage_manifest"
fi

readonly profdata="$TEST_TMP_DIR/coverage.profdata"
xcrun llvm-profdata merge "$profraw" --output "$profdata"
xcrun llvm-cov \
  export \
  -format lcov \
  -instr-profile "$profdata" \
  -ignore-filename-regex='.*external/.+' \
  -path-equivalence=".,$PWD" \
  "$test_binary" \
  @"$llvm_coverage_manifest" \
  > "$COVERAGE_OUTPUT_FILE"

if [[ -f "${TEST_PREMATURE_EXIT_FILE:-}" ]]; then
  rm -f "$TEST_PREMATURE_EXIT_FILE"
fi
