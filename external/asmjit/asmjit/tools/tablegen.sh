#!/usr/bin/env sh
set -e
node ./tablegen-a32.js $@
node ./tablegen-a64.js $@
node ./tablegen-x86.js $@
