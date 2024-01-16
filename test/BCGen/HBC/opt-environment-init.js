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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(o: any): object
// CHECK-NEXT:frame = [cnt: number, flag: undefined|boolean, flag1: undefined|number, flag2: undefined|number]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [flag]: undefined|boolean
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [flag1]: undefined|number
// CHECK-NEXT:       StoreFrameInst 0: number, [cnt]: number
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [flag2]: undefined|number
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %""(): functionCode
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end

// CHECK:function ""(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:undefined|boolean) [flag@foo]: undefined|boolean
// CHECK-NEXT:       CondBranchInst %0: undefined|boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst (:undefined|boolean) %0: undefined|boolean, %BB0, true: boolean, %BB2
// CHECK-NEXT:       StoreFrameInst %3: undefined|boolean, [flag@foo]: undefined|boolean
// CHECK-NEXT:  %5 = LoadFrameInst (:undefined|number) [flag1@foo]: undefined|number
// CHECK-NEXT:       CondBranchInst %5: undefined|number, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = PhiInst (:undefined|number) %5: undefined|number, %BB1, 1: number, %BB4
// CHECK-NEXT:       StoreFrameInst %8: undefined|number, [flag1@foo]: undefined|number
// CHECK-NEXT:  %10 = LoadFrameInst (:undefined|number) [flag2@foo]: undefined|number
// CHECK-NEXT:        CondBranchInst %10: undefined|number, %BB5, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = PhiInst (:undefined|number) %10: undefined|number, %BB3, 2: number, %BB6
// CHECK-NEXT:        StoreFrameInst %13: undefined|number, [flag2@foo]: undefined|number
// CHECK-NEXT:  %15 = LoadFrameInst (:number) [cnt@foo]: number
// CHECK-NEXT:  %16 = FAddInst (:number) %15: number, 1: number
// CHECK-NEXT:        StoreFrameInst %16: number, [cnt@foo]: number
// CHECK-NEXT:        ReturnInst %16: number
// CHECK-NEXT:function_end

// CHKLIR:function global(): undefined
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:       DeclareGlobalVarInst "foo": string
// CHKLIR-NEXT:  %1 = HBCCreateEnvironmentInst (:any)
// CHKLIR-NEXT:  %2 = HBCCreateFunctionInst (:object) %foo(): functionCode, %1: any
// CHKLIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKLIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "foo": string
// CHKLIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:       ReturnInst %5: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function foo(o: any): object
// CHKLIR-NEXT:frame = [cnt: number, flag: undefined|boolean, flag1: undefined|number, flag2: undefined|number]
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCCreateEnvironmentInst (:any)
// CHKLIR-NEXT:  %1 = HBCLoadConstInst (:number) 0: number
// CHKLIR-NEXT:       HBCStoreToEnvironmentInst %0: any, %1: number, [cnt]: number
// CHKLIR-NEXT:  %3 = LoadParamInst (:any) %o: any
// CHKLIR-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:  %5 = HBCCallNInst (:any) %3: any, empty: any, empty: any, %4: undefined, %4: undefined
// CHKLIR-NEXT:       HBCStoreToEnvironmentInst %0: any, %4: undefined, [flag2]: undefined|number
// CHKLIR-NEXT:  %7 = HBCCreateFunctionInst (:object) %""(): functionCode, %0: any
// CHKLIR-NEXT:       ReturnInst %7: object
// CHKLIR-NEXT:function_end

// CHKLIR:function ""(): number
// CHKLIR-NEXT:frame = []
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = HBCResolveEnvironment (:any) %foo(): any
// CHKLIR-NEXT:  %1 = HBCLoadFromEnvironmentInst (:undefined|boolean) %0: any, [flag@foo]: undefined|boolean
// CHKLIR-NEXT:       CondBranchInst %1: undefined|boolean, %BB1, %BB2
// CHKLIR-NEXT:%BB2:
// CHKLIR-NEXT:  %3 = HBCLoadConstInst (:boolean) true: boolean
// CHKLIR-NEXT:       BranchInst %BB1
// CHKLIR-NEXT:%BB1:
// CHKLIR-NEXT:  %5 = PhiInst (:undefined|boolean) %1: undefined|boolean, %BB0, %3: boolean, %BB2
// CHKLIR-NEXT:       HBCStoreToEnvironmentInst %0: any, %5: undefined|boolean, [flag@foo]: undefined|boolean
// CHKLIR-NEXT:  %7 = HBCLoadFromEnvironmentInst (:undefined|number) %0: any, [flag1@foo]: undefined|number
// CHKLIR-NEXT:       CondBranchInst %7: undefined|number, %BB3, %BB4
// CHKLIR-NEXT:%BB4:
// CHKLIR-NEXT:  %9 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:        BranchInst %BB3
// CHKLIR-NEXT:%BB3:
// CHKLIR-NEXT:  %11 = PhiInst (:undefined|number) %7: undefined|number, %BB1, %9: number, %BB4
// CHKLIR-NEXT:        HBCStoreToEnvironmentInst %0: any, %11: undefined|number, [flag1@foo]: undefined|number
// CHKLIR-NEXT:  %13 = HBCLoadFromEnvironmentInst (:undefined|number) %0: any, [flag2@foo]: undefined|number
// CHKLIR-NEXT:        CondBranchInst %13: undefined|number, %BB5, %BB6
// CHKLIR-NEXT:%BB6:
// CHKLIR-NEXT:  %15 = HBCLoadConstInst (:number) 2: number
// CHKLIR-NEXT:        BranchInst %BB5
// CHKLIR-NEXT:%BB5:
// CHKLIR-NEXT:  %17 = PhiInst (:undefined|number) %13: undefined|number, %BB3, %15: number, %BB6
// CHKLIR-NEXT:        HBCStoreToEnvironmentInst %0: any, %17: undefined|number, [flag2@foo]: undefined|number
// CHKLIR-NEXT:  %19 = HBCLoadFromEnvironmentInst (:number) %0: any, [cnt@foo]: number
// CHKLIR-NEXT:  %20 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:  %21 = FAddInst (:number) %19: number, %20: number
// CHKLIR-NEXT:        HBCStoreToEnvironmentInst %0: any, %21: number, [cnt@foo]: number
// CHKLIR-NEXT:        ReturnInst %21: number
// CHKLIR-NEXT:function_end
