/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -dump-ir %s -O

function simple_test0(x, y) {
  return x instanceof y;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simple_test0": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %simple_test0(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "simple_test0": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: any, y: any]

// CHECK:function simple_test0(x: any, y: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS1.y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS1.y]: any
// CHECK-NEXT:  %8 = BinaryInstanceOfInst (:boolean) %6: any, %7: any
// CHECK-NEXT:       ReturnInst %8: boolean
// CHECK-NEXT:function_end
