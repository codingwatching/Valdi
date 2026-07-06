#!/usr/bin/env bash

set -eux

(

# Intended to be run from open_source/
cd "$(dirname "$0")/../.."

bzl test //valdi:test_snap_drawing //valdi:test_hermes --test_output=errors
bzl test //valdi:test_layout --test_output=all --test_arg=--gtest_print_time=1

if [[ $(uname) != Linux ]] ; then
    bzl test //valdi:valdi_ios_objc_test
    bzl test //valdi:valdi_ios_swift_test
    bzl test //valdi:valdi_macos_objc_test
fi

)
