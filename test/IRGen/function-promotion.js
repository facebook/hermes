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
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = [x: any, y: any, z: any, x#1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [z]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x#1]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %x(): any
// CHECK-NEXT:       StoreFrameInst %5: object, [x#1]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %y(): any
// CHECK-NEXT:       StoreFrameInst %7: object, [y]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %z(): any
// CHECK-NEXT:        StoreFrameInst %9: object, [z]: any
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %15 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %12: any, %13: any, %14: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function x(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function y(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function z(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
