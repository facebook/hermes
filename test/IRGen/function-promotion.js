/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo() {
  let x;
  var y;
  {
    function x() {}
    function y() {}
    function z() {}
  }
  // x is not promoted.
  // y is promoted.
  // z is promoted.
  print(x, y, z);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst (:any) %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = [x: any, y: any, z: any, x#1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %1 = StoreFrameInst undefined: undefined, [y]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %x(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: closure, [x#1]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %y(): any
// CHECK-NEXT:  %6 = StoreFrameInst %5: closure, [y]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %z(): any
// CHECK-NEXT:  %8 = StoreFrameInst %7: closure, [z]: any
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %13 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, %10: any, %11: any, %12: any
// CHECK-NEXT:  %14 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function x(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function y(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function z(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
