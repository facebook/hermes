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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = [x: any, y: any, z: any, x#1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [z]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x#1]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %x(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [x#1]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %1: environment, %y(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [y]: any
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %1: environment, %z(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [z]: any
// CHECK-NEXT:  %13 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [z]: any
// CHECK-NEXT:  %17 = CallInst (:any) %13: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %14: any, %15: any, %16: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function x(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %foo(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %x(): any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function y(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %foo(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %y(): any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function z(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %foo(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %z(): any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
