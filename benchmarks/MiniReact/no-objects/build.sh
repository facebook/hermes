#!/bin/sh

REACTMINI_ROOT=$(dirname -- "$( readlink -f -- "$0"; )")
REACTMINI_SRC_ROOT="$REACTMINI_ROOT/src"
REACTMINI_OUT_ROOT="$REACTMINI_ROOT/out"
BENCHMARKS_ROOT="$REACTMINI_ROOT/../.."

ENTRYPOINTS=(
  simple
  music
)

for ENTRYPOINT in "${ENTRYPOINTS[@]}"; do
  $BENCHMARKS_ROOT/build-helpers/flow-bundler/bin/flow-bundler \
    --root $REACTMINI_SRC_ROOT \
    --out $REACTMINI_OUT_ROOT/$ENTRYPOINT.js \
    --es5 \
    --strip-types\
    ./app/$ENTRYPOINT/index.js
done
