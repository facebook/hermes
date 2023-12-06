# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# RUN: bash %s %S %shermes | %FileCheck --match-full-lines %s
# REQUIRES: !fbcode_coverage
# shellcheck shell=bash

# Test compiler messages with a source map.

set -e

SRCDIR=$1
SHERMES=$2

cd "$SRCDIR"

$SHERMES -typed -dump-ir --source-map=prog1.js.map prog1.js 2>&1 || true
# CHECK:      prog1/mod1.js:9:5: [original] error: ft: return value incompatible with return type
# CHECK-NEXT: prog1.js:15:3: [transpiled] error: ft: return value incompatible with return type
# CHECK-NEXT:   return 10;
# CHECK-NEXT:   ^~~~~~~~~~
# CHECK-NEXT: prog1/index.js:10:7: [original] error: ft: incompatible binary operation: + cannot be applied to string and number
# CHECK-NEXT: prog1.js:18:7: [transpiled] error: ft: incompatible binary operation: + cannot be applied to string and number
# CHECK-NEXT: print(mod1$foo() + 1);
# CHECK-NEXT:       ^~~~~~~~~~~~~~
# CHECK-NEXT: Emitted 2 errors. exiting.
