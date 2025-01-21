/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -dump-ir -Xes6-block-scoping %s | %FileCheckOrRegen --match-full-lines %s

var arr = [];

// Worst possible case: every expression captures.
function foo_full() {
    for(let i = 0; arr.push(()=>i), ++i < 10; arr.push(()=>i), i += 2) {
        arr.push(()=>i);
        print(i);
    }
}

// Test expression doesn't capture. This is the same as "full".
function foo_testnc() {
    for(let i = 0; ++i < 10; arr.push(()=>i), i += 2) {
        arr.push(()=>i);
        print(i);
    }
}

// Update expression doesn't capture.
function foo_updatenc() {
    for(let i = 0; arr.push(()=>i), ++i < 10; i += 2) {
        arr.push(()=>i);
        print(i);
    }
}

// Both test and update expressions don't capture.
function foo_testnc_updatenc() {
    for(let i = 0; ++i < 10; i += 2) {
        arr.push(()=>i);
        print(i);
    }
}

// Nothing captures. This should be very similar to the var case, except for TDZ.
function foo_allnc() {
    for(let i = 0; ++i < 10; i += 2) {
        print(i);
    }
}

// The init statement captures.
function foo_init_capture(){
    for (let i = 0, x = ()=>i; i < 3; ++i){
        print(x());
    }
}

// The var case, which produces the best possible code.
function foo_var() {
    for(var i = 0; ++i < 10; i += 2) {
        print(i);
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "arr": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_full": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_testnc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_updatenc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_testnc_updatenc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_allnc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_init_capture": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_var": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %foo_full(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "foo_full": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %foo_testnc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "foo_testnc": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %foo_updatenc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "foo_updatenc": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %foo_testnc_updatenc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "foo_testnc_updatenc": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %foo_allnc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "foo_allnc": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %0: environment, %foo_init_capture(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "foo_init_capture": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %0: environment, %foo_var(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "foo_var": string
// CHECK-NEXT:  %23 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:        StorePropertyLooseInst %23: object, globalObject: object, "arr": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [i: number]

// CHECK:function foo_full(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst (:boolean) true: boolean, %BB0, false: boolean, %BB4
// CHECK-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %12: number, %BB4
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %4: environment, %3: number, [%VS1.i]: number
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = PhiInst (:number) %3: number, %BB1, %20: number, %BB3
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "push": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %4: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %11 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, %8: any, %10: object
// CHECK-NEXT:  %12 = FAddInst (:number) %7: number, 1: number
// CHECK-NEXT:        StoreFrameInst %4: environment, %12: number, [%VS1.i]: number
// CHECK-NEXT:  %14 = FLessThanInst (:boolean) %12: number, 10: number
// CHECK-NEXT:        CondBranchInst %14: boolean, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %17 = LoadPropertyInst (:any) %16: any, "push": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %4: environment, %""(): functionCode
// CHECK-NEXT:  %19 = CallInst (:any) %17: any, empty: any, false: boolean, empty: any, undefined: undefined, %16: any, %18: object
// CHECK-NEXT:  %20 = FAddInst (:number) %3: number, 2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, %20: number, [%VS1.i]: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %23 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %24 = LoadPropertyInst (:any) %23: any, "push": string
// CHECK-NEXT:  %25 = CreateFunctionInst (:object) %4: environment, %" 2#"(): functionCode
// CHECK-NEXT:  %26 = CallInst (:any) %24: any, empty: any, false: boolean, empty: any, undefined: undefined, %23: any, %25: object
// CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %28 = CallInst (:any) %27: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %12: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [i: number]

// CHECK:function foo_testnc(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst (:boolean) true: boolean, %BB0, false: boolean, %BB4
// CHECK-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %8: number, %BB4
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %4: environment, %3: number, [%VS2.i]: number
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = PhiInst (:number) %3: number, %BB1, %16: number, %BB3
// CHECK-NEXT:  %8 = FAddInst (:number) %7: number, 1: number
// CHECK-NEXT:       StoreFrameInst %4: environment, %8: number, [%VS2.i]: number
// CHECK-NEXT:  %10 = FLessThanInst (:boolean) %8: number, 10: number
// CHECK-NEXT:        CondBranchInst %10: boolean, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %13 = LoadPropertyInst (:any) %12: any, "push": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %4: environment, %" 3#"(): functionCode
// CHECK-NEXT:  %15 = CallInst (:any) %13: any, empty: any, false: boolean, empty: any, undefined: undefined, %12: any, %14: object
// CHECK-NEXT:  %16 = FAddInst (:number) %3: number, 2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, %16: number, [%VS2.i]: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %19: any, "push": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %4: environment, %" 4#"(): functionCode
// CHECK-NEXT:  %22 = CallInst (:any) %20: any, empty: any, false: boolean, empty: any, undefined: undefined, %19: any, %21: object
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %24 = CallInst (:any) %23: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %8: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [i: number]

// CHECK:function foo_updatenc(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst (:boolean) true: boolean, %BB0, false: boolean, %BB4
// CHECK-NEXT:  %3 = PhiInst (:number) 0: number, %BB0, %12: number, %BB4
// CHECK-NEXT:  %4 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %4: environment, %3: number, [%VS3.i]: number
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = PhiInst (:number) %3: number, %BB1, %16: number, %BB3
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "push": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %4: environment, %" 5#"(): functionCode
// CHECK-NEXT:  %11 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, %8: any, %10: object
// CHECK-NEXT:  %12 = FAddInst (:number) %7: number, 1: number
// CHECK-NEXT:        StoreFrameInst %4: environment, %12: number, [%VS3.i]: number
// CHECK-NEXT:  %14 = FLessThanInst (:boolean) %12: number, 10: number
// CHECK-NEXT:        CondBranchInst %14: boolean, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = FAddInst (:number) %3: number, 2: number
// CHECK-NEXT:        StoreFrameInst %4: environment, %16: number, [%VS3.i]: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %19: any, "push": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %4: environment, %" 6#"(): functionCode
// CHECK-NEXT:  %22 = CallInst (:any) %20: any, empty: any, false: boolean, empty: any, undefined: undefined, %19: any, %21: object
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %24 = CallInst (:any) %23: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %12: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [i: number]

// CHECK:function foo_testnc_updatenc(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = PhiInst (:number) 1: number, %BB0, %12: number, %BB1
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %2: number, [%VS4.i]: number
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "push": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %3: environment, %" 7#"(): functionCode
// CHECK-NEXT:  %8 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, %5: any, %7: object
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: number
// CHECK-NEXT:  %11 = FAddInst (:number) %2: number, 2: number
// CHECK-NEXT:  %12 = FAddInst (:number) %11: number, 1: number
// CHECK-NEXT:  %13 = FLessThanInst (:boolean) %12: number, 10: number
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo_allnc(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:number) 1: number, %BB0, %5: number, %BB1
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %1: number
// CHECK-NEXT:  %4 = FAddInst (:number) %1: number, 2: number
// CHECK-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHECK-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 10: number
// CHECK-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo_init_capture(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:boolean) true: boolean, %BB0, false: boolean, %BB4
// CHECK-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %4: number, %BB4
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = PhiInst (:number) %2: number, %BB1, %7: number, %BB3
// CHECK-NEXT:  %5 = FLessThanInst (:boolean) %4: number, 3: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = FAddInst (:number) %2: number, 1: number
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, 0: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo_var(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:number) 1: number, %BB0, %5: number, %BB1
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %1: number
// CHECK-NEXT:  %4 = FAddInst (:number) %1: number, 2: number
// CHECK-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHECK-NEXT:  %6 = FLessThanInst (:boolean) %5: number, 10: number
// CHECK-NEXT:       CondBranchInst %6: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:arrow ""(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS1.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:arrow " 1#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS1.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:arrow " 2#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS1.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:arrow " 3#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS2.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:arrow " 4#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS2.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:arrow " 5#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS3.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:arrow " 6#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS3.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end

// CHECK:arrow " 7#"(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:number) %0: environment, [%VS4.i]: number
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
