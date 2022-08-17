/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ( ! %hermesc -funsafe-intrinsics -dump-lir -Werror %s 2>&1 ) | %FileCheck %s --match-full-lines
// REQUIRES: run_wasm

function wrongNumArgs(func) {
  return __uasm.add32(1, 2, 3);
}
//CHECK: {{.*}}unsafe-intrinsics-wrong-num-args.js:11:1: error: the intrinsic "add32" is called with incorrect number of arguments. Expecting 2 but got 3.
//CHECK-NEXT: function wrongNumArgs(func) {
//CHECK-NEXT: ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//CHECK-NEXT: Emitted 1 errors. exiting.
