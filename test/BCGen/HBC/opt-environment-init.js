/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -O -dump-lir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKLIR %s
//
// Test optimizing out of unnecessary store undefined into a freshly created
// environment

function foo(o) {
    var cnt = 0;
    var flag, flag1;
    var flag2;
    o();
    // This assignment shouldn't be optimized out.
    flag2 = undefined;
    return function () {
        flag = flag || true;
        flag1 = flag1 || 1;
        flag2 = flag2 || 2;
        return ++cnt;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [cnt: number, flag: undefined|boolean, flag1: undefined|number, flag2: undefined|number]

// CHECK:function foo(o: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.flag]: undefined|boolean
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.flag1]: undefined|number
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.flag2]: undefined|number
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS1.cnt]: number
// CHECK-NEXT:  %7 = CallInst (:any) %2: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.flag2]: undefined|number
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:        ReturnInst %9: object
// CHECK-NEXT:function_end

// CHECK:function ""(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:undefined|boolean) %0: environment, [%VS1.flag]: undefined|boolean
// CHECK-NEXT:       CondBranchInst %1: undefined|boolean, %BB2, %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = PhiInst (:undefined|boolean) %1: undefined|boolean, %BB0, true: boolean, %BB1
// CHECK-NEXT:       StoreFrameInst %0: environment, %4: undefined|boolean, [%VS1.flag]: undefined|boolean
// CHECK-NEXT:  %6 = LoadFrameInst (:undefined|number) %0: environment, [%VS1.flag1]: undefined|number
// CHECK-NEXT:       CondBranchInst %6: undefined|number, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = PhiInst (:undefined|number) %6: undefined|number, %BB2, 1: number, %BB3
// CHECK-NEXT:        StoreFrameInst %0: environment, %9: undefined|number, [%VS1.flag1]: undefined|number
// CHECK-NEXT:  %11 = LoadFrameInst (:undefined|number) %0: environment, [%VS1.flag2]: undefined|number
// CHECK-NEXT:        CondBranchInst %11: undefined|number, %BB6, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %14 = PhiInst (:undefined|number) %11: undefined|number, %BB4, 2: number, %BB5
// CHECK-NEXT:        StoreFrameInst %0: environment, %14: undefined|number, [%VS1.flag2]: undefined|number
// CHECK-NEXT:  %16 = LoadFrameInst (:number) %0: environment, [%VS1.cnt]: number
// CHECK-NEXT:  %17 = FAddInst (:number) %16: number, 1: number
// CHECK-NEXT:        StoreFrameInst %0: environment, %17: number, [%VS1.cnt]: number
// CHECK-NEXT:        ReturnInst %17: number
// CHECK-NEXT:function_end

// CHKLIR:scope %VS0 []

// CHKLIR:function global(): undefined
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:       DeclareGlobalVarInst "foo": string
// CHKLIR-NEXT:  %1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKLIR-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHKLIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKLIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "foo": string
// CHKLIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:       ReturnInst %5: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:scope %VS1 [cnt: number, flag: undefined|boolean, flag1: undefined|number, flag2: undefined|number]

// CHKLIR:function foo(o: any): object
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateFunctionEnvironmentInst (:environment) %VS1: any, %parentScope: environment
// CHKLIR-NEXT:  %1 = HBCLoadConstInst (:number) 0: number
// CHKLIR-NEXT:       StoreFrameInst %0: environment, %1: number, [%VS1.cnt]: number
// CHKLIR-NEXT:  %3 = LoadParamInst (:any) %o: any
// CHKLIR-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:  %5 = HBCCallNInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, %4: undefined
// CHKLIR-NEXT:       StoreFrameInst %0: environment, %4: undefined, [%VS1.flag2]: undefined|number
// CHKLIR-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHKLIR-NEXT:       ReturnInst %7: object
// CHKLIR-NEXT:function_end

// CHKLIR:function ""(): number
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHKLIR-NEXT:  %1 = LoadFrameInst (:undefined|boolean) %0: environment, [%VS1.flag]: undefined|boolean
// CHKLIR-NEXT:       CondBranchInst %1: undefined|boolean, %BB2, %BB1
// CHKLIR-NEXT:%BB1:
// CHKLIR-NEXT:  %3 = HBCLoadConstInst (:boolean) true: boolean
// CHKLIR-NEXT:       BranchInst %BB2
// CHKLIR-NEXT:%BB2:
// CHKLIR-NEXT:  %5 = PhiInst (:undefined|boolean) %1: undefined|boolean, %BB0, %3: boolean, %BB1
// CHKLIR-NEXT:       StoreFrameInst %0: environment, %5: undefined|boolean, [%VS1.flag]: undefined|boolean
// CHKLIR-NEXT:  %7 = LoadFrameInst (:undefined|number) %0: environment, [%VS1.flag1]: undefined|number
// CHKLIR-NEXT:       CondBranchInst %7: undefined|number, %BB4, %BB3
// CHKLIR-NEXT:%BB3:
// CHKLIR-NEXT:  %9 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:        BranchInst %BB4
// CHKLIR-NEXT:%BB4:
// CHKLIR-NEXT:  %11 = PhiInst (:undefined|number) %7: undefined|number, %BB2, %9: number, %BB3
// CHKLIR-NEXT:        StoreFrameInst %0: environment, %11: undefined|number, [%VS1.flag1]: undefined|number
// CHKLIR-NEXT:  %13 = LoadFrameInst (:undefined|number) %0: environment, [%VS1.flag2]: undefined|number
// CHKLIR-NEXT:        CondBranchInst %13: undefined|number, %BB6, %BB5
// CHKLIR-NEXT:%BB5:
// CHKLIR-NEXT:  %15 = HBCLoadConstInst (:number) 2: number
// CHKLIR-NEXT:        BranchInst %BB6
// CHKLIR-NEXT:%BB6:
// CHKLIR-NEXT:  %17 = PhiInst (:undefined|number) %13: undefined|number, %BB4, %15: number, %BB5
// CHKLIR-NEXT:        StoreFrameInst %0: environment, %17: undefined|number, [%VS1.flag2]: undefined|number
// CHKLIR-NEXT:  %19 = LoadFrameInst (:number) %0: environment, [%VS1.cnt]: number
// CHKLIR-NEXT:  %20 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:  %21 = FAddInst (:number) %19: number, %20: number
// CHKLIR-NEXT:        StoreFrameInst %0: environment, %21: number, [%VS1.cnt]: number
// CHKLIR-NEXT:        ReturnInst %21: number
// CHKLIR-NEXT:function_end
