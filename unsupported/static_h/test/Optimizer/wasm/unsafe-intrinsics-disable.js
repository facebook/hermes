/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-lir %s | %FileCheck --match-full-lines %s
// REQUIRES: run_wasm

// LowerIntrinsics pass is only enabled if -funsafe-intrinsics is set.
function undefinedIntrinsic(func) {
  return __uasm.foo(42, 3);
}
//CHECK-LABEL:function undefinedIntrinsic(func)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = HBCGetGlobalObjectInst
//CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst %0 : object, "__uasm" : string
//CHECK-NEXT:  %2 = LoadPropertyInst %1, "foo" : string
//CHECK-NEXT:  %3 = HBCLoadConstInst 42 : number
//CHECK-NEXT:  %4 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  %5 = HBCCallNInst %2, %1, %3 : number, %4 : number
//CHECK-NEXT:  %6 = ReturnInst %5
//CHECK-NEXT:function_end
