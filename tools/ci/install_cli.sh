#!/usr/bin/env bash

set -eux

(

# Intended to be run from open_source/
cd "$(dirname "$0")/../../npm_modules/cli"

npm run cli:install

# Make sure this completes successfully
valdi --help

)
