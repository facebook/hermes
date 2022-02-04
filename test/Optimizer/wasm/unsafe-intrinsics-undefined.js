/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ( ! %hermesc -funsafe-intrinsics -dump-lir -Werror %s 2>&1 ) | %FileCheck %s --match-full-lines
// REQUIRES: run_wasm

function undefinedIntrinsic(func) {
  return __uasm.foo(42, 3);
}
//CHECK: {{.*}}unsafe-intrinsics-undefined.js:11:1: error: the intrinsic "foo" is undefined.
//CHECK-NEXT: function undefinedIntrinsic(func) {
//CHECK-NEXT: ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//CHECK-NEXT: Emitted 1 errors. exiting.
