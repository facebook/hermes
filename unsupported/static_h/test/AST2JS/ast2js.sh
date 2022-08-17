#!/bin/sh
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# RUN: sh %s %hermesc %S/input1.js.txt %t-input1 %S/input1-p.js.txt
# RUN: sh %s %hermesc %S/richards.js.txt %t-richards

set -e

HERMESC=$1
INP=$2
TMPN=$3
PRETTY=$4

"$HERMESC" -dump-ast -Xinclude-raw-ast-prop=0 "$INP" > "${TMPN}.json"

"$HERMESC" -dump-js -pretty "$INP" > "${TMPN}-p.js"
if [ -n "$PRETTY" ]; then
    cmp "$PRETTY" "${TMPN}-p.js"
fi
"$HERMESC" -dump-ast -Xinclude-raw-ast-prop=0 "${TMPN}-p.js" > "${TMPN}-p.json"
cmp "${TMPN}.json" "${TMPN}-p.json"

"$HERMESC" -dump-js -pretty=0 "$INP" > "${TMPN}-m.js"
"$HERMESC" -dump-ast -Xinclude-raw-ast-prop=0 "${TMPN}-m.js" > "${TMPN}-m.json"
cmp "${TMPN}.json" "${TMPN}-m.json"
