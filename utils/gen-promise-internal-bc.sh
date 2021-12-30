#!/usr/bin/env sh
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# This script generated the `lib/InternalBytecode/Promise.js` by performing the
# below steps:
#
# 0. It copies the `util/promise/` to a tmp dir to run `yarn` and `rollup`. The
#   tmp dir is always wiped away on Bash's trap exit so we can keep our source 
#   tree clean regardless of whether the script was success or not.
#
# 1. The Rollup is configured to build the `util/promise/index.js`, which
#   imports the `promise` polyfill that we installed with `yarn`, exposes it to
#   Hermes's globalThis, and then setup Hermes promise rejection tracking.
#
# 2. It adds `@nolint` to the start of the file and append a snippet to make 
#   the initialization of Promise conditionalized against 
#   `HermesInternal.hasPromise()` to ensure there is no runtime overhead when
#   Hermes' Promise flag is disabled.
#
# 3. Finally, it moved the resulted file to the `InternalBytecode`.


SCRIPT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

BC_DIR="${SCRIPT}/../lib/InternalBytecode"
TMP_DIR=$(mktemp -d)
PROMISE_DIR="${SCRIPT}/promise"
TMP_PROMISE_DIR="${TMP_DIR}/promise"

TMP_PROMISE_JS="${TMP_PROMISE_DIR}/Promise.js"
BC_PROMISE_JS="${BC_DIR}/01-Promise.js"

NOLINT="/* @nolint */"
PATCH1="  var useEngineQueue = HermesInternal.useEngineQueue()\;"
PATCH2='(useEngineQueue ? HermesInternal.enqueueJob : setImmediate)'

gen() {
  # It's tricky to ensure `sed -i` work on both GNU and macOS consistently.
  # https://stackoverflow.com/questions/5694228/sed-in-place-flag-that-works-both-on-mac-bsd-and-linux
  sed -i.bak "1s;^;${NOLINT};" $TMP_PROMISE_JS && \
  sed -i.bak "3s;^;${PATCH1};" $TMP_PROMISE_JS && \
  sed -i.bak "s/setImmediate/${PATCH2}/g" $TMP_PROMISE_JS && \
  sed -i.bak '$ d' $TMP_PROMISE_JS && \
  cat <<EOT >> $TMP_PROMISE_JS
});
if (HermesInternal?.hasPromise?.()) {
  initPromise();
}
EOT
}

cleanup() {
  rm -rf $TMP_DIR
}

trap cleanup EXIT
cp -r $PROMISE_DIR $TMP_DIR
cd $TMP_PROMISE_DIR
yarn && yarn start && gen && mv $TMP_PROMISE_JS $BC_PROMISE_JS
