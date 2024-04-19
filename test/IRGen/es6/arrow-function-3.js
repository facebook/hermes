/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(x = () => this) {
    return x();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [?anon_0_this: any, ?anon_1_new.target: undefined|object, x: any]

// CHECK:function foo(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS1: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS1.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS1.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, undefined: undefined, [%VS1.x]: any
// CHECK-NEXT:  %8 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %9 = BinaryStrictlyNotEqualInst (:any) %8: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %9: any, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %3: environment, %x(): functionCode
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = PhiInst (:any) %8: any, %BB0, %11: object, %BB1
// CHECK-NEXT:        StoreFrameInst %3: environment, %13: any, [%VS1.x]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %3: environment, [%VS1.x]: any
// CHECK-NEXT:  %16 = CallInst (:any) %15: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        ReturnInst %16: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:arrow x(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [%VS1.?anon_0_this]: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end
