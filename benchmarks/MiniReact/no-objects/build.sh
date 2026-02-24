#!/bin/sh

REACTMINI_ROOT=$(dirname -- "$( readlink -f -- "$0"; )")
BENCHMARKS_ROOT="$REACTMINI_ROOT/../.."


$BENCHMARKS_ROOT/build-helpers/flow-bundler/bin/flow-bundler \
  -c $REACTMINI_ROOT/build.config.js
