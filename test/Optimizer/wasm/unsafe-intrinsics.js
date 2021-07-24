/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -funsafe-intrinsics -dump-lir %s | %FileCheck --match-full-lines %s

// Instrinsics that are defined should not cause any error. But
// we also do not do anything, yet.
function unsafeIntrinsics(func) {
  t0 = __uasm.add32(1, 2);
  t1 = __uasm.modu32(42, 7);
  return t0 + t1;
}
//CHECK-LABEL:function unsafeIntrinsics(func) : string|number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = HBCGetGlobalObjectInst
//CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst %0 : object, "__uasm" : string
//CHECK-NEXT:  %2 = LoadPropertyInst %1, "add32" : string
//CHECK-NEXT:  %3 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  %4 = HBCLoadConstInst 2 : number
//CHECK-NEXT:  %5 = HBCCallNInst %2, %1, %3 : number, %4 : number
//CHECK-NEXT:  %6 = StorePropertyInst %5, %0 : object, "t0" : string
//CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst %0 : object, "__uasm" : string
//CHECK-NEXT:  %8 = LoadPropertyInst %7, "modu32" : string
//CHECK-NEXT:  %9 = HBCLoadConstInst 42 : number
//CHECK-NEXT:  %10 = HBCLoadConstInst 7 : number
//CHECK-NEXT:  %11 = HBCCallNInst %8, %7, %9 : number, %10 : number
//CHECK-NEXT:  %12 = StorePropertyInst %11, %0 : object, "t1" : string
//CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst %0 : object, "t0" : string
//CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst %0 : object, "t1" : string
//CHECK-NEXT:  %15 = BinaryOperatorInst '+', %13, %14
//CHECK-NEXT:  %16 = ReturnInst %15 : string|number
//CHECK-NEXT:function_end
