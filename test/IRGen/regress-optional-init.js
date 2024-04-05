/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
//
// Make sure that the optional initialization generates a correct Phi node, when the initialization
// is multi-block.

var a, b;
function foo(param = a || b) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "a": string
// CHECK-NEXT:       DeclareGlobalVarInst "b": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [param: any]

// CHECK:function foo(param: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.param]: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %param: any
// CHECK-NEXT:  %4 = BinaryStrictlyNotEqualInst (:any) %3: any, undefined: undefined
// CHECK-NEXT:       CondBranchInst %4: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_logical: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:       StoreStackInst %7: any, %6: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB4, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = PhiInst (:any) %3: any, %BB0, %16: any, %BB4
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [%VS1.param]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:        StoreStackInst %13: any, %6: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %6: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
