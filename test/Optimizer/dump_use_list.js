/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -hermes-parser -dump-ir %s -dump-instr-uselist  | %FileCheckOrRegen %s --match-full-lines

function foo(a, b) {
  var c = a + b;
  return c * c;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any // users: %2
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode // users: %3
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any // users: %5 %6
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any // users: %7
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment // users: %1
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment // users: %3 %5 %6 %7 %8 %10 %11 %12
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any // users: %3
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any // users: %5
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [b]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [c]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [a]: any // users: %9
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [b]: any // users: %9
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %7: any, %8: any // users: %10
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: any, [c]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [c]: any // users: %13
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [c]: any // users: %13
// CHECK-NEXT:  %13 = BinaryMultiplyInst (:any) %11: any, %12: any // users: %14
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:function_end
