/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s -funsafe-intrinsics | %FileCheck --match-full-lines %s
// REQUIRES: run_wasm

// Only one copy of AsInt32Inst should be shown.
function test_redundant(x) {
  x = (((x | 0) | 0) | 0) | 0;
}

//CHECK-LABEL:function test_redundant(x) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AsInt32Inst %x
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
