/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Ensure that we can use "arguments" when initializing formal parameters.

function foo(a = arguments) {
    return a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [a: any]

// CHECK:function foo(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS1: any, %1: environment
// CHECK-NEXT:       StoreFrameInst %2: environment, undefined: undefined, [%VS1.a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %5 = BinaryStrictlyNotEqualInst (:any) %4: any, undefined: undefined
// CHECK-NEXT:       CondBranchInst %5: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = PhiInst (:any) %4: any, %BB0, %0: object, %BB1
// CHECK-NEXT:       StoreFrameInst %2: environment, %8: any, [%VS1.a]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %2: environment, [%VS1.a]: any
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end
