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
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo_full(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "foo_full": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo_testnc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "foo_testnc": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo_updatenc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "foo_updatenc": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo_testnc_updatenc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "foo_testnc_updatenc": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo_allnc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "foo_allnc": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo_init_capture(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "foo_init_capture": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo_var(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "foo_var": string
// CHECK-NEXT:  %23 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:        StorePropertyLooseInst %23: object, globalObject: object, "arr": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [i: number]

// CHECK:function foo_full(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:boolean) true: boolean, %BB0, false: boolean, %BB4
// CHECK-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %11: number, %BB4
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:       StoreFrameInst %3: environment, %2: number, [%VS1.i]: number
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = PhiInst (:number) %2: number, %BB1, %19: number, %BB3
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "push": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %3: environment, %VS1: any, %" 1#"(): functionCode
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %7: any, %9: object
// CHECK-NEXT:  %11 = FAddInst (:number) %6: number, 1: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %11: number, [%VS1.i]: number
// CHECK-NEXT:  %13 = FLessThanInst (:boolean) %11: number, 10: number
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %16 = LoadPropertyInst (:any) %15: any, "push": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %3: environment, %VS1: any, %""(): functionCode
// CHECK-NEXT:  %18 = CallInst (:any) %16: any, empty: any, false: boolean, empty: any, undefined: undefined, %15: any, %17: object
// CHECK-NEXT:  %19 = FAddInst (:number) %2: number, 2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %19: number, [%VS1.i]: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %23 = LoadPropertyInst (:any) %22: any, "push": string
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %3: environment, %VS1: any, %" 2#"(): functionCode
// CHECK-NEXT:  %25 = CallInst (:any) %23: any, empty: any, false: boolean, empty: any, undefined: undefined, %22: any, %24: object
// CHECK-NEXT:  %26 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %27 = CallInst (:any) %26: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %11: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [i: number]

// CHECK:function foo_testnc(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:boolean) true: boolean, %BB0, false: boolean, %BB4
// CHECK-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %7: number, %BB4
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS2: any, empty: any
// CHECK-NEXT:       StoreFrameInst %3: environment, %2: number, [%VS2.i]: number
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = PhiInst (:number) %2: number, %BB1, %15: number, %BB3
// CHECK-NEXT:  %7 = FAddInst (:number) %6: number, 1: number
// CHECK-NEXT:       StoreFrameInst %3: environment, %7: number, [%VS2.i]: number
// CHECK-NEXT:  %9 = FLessThanInst (:boolean) %7: number, 10: number
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %11: any, "push": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %3: environment, %VS2: any, %" 3#"(): functionCode
// CHECK-NEXT:  %14 = CallInst (:any) %12: any, empty: any, false: boolean, empty: any, undefined: undefined, %11: any, %13: object
// CHECK-NEXT:  %15 = FAddInst (:number) %2: number, 2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %15: number, [%VS2.i]: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %19 = LoadPropertyInst (:any) %18: any, "push": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %3: environment, %VS2: any, %" 4#"(): functionCode
// CHECK-NEXT:  %21 = CallInst (:any) %19: any, empty: any, false: boolean, empty: any, undefined: undefined, %18: any, %20: object
// CHECK-NEXT:  %22 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %23 = CallInst (:any) %22: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %7: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [i: number]

// CHECK:function foo_updatenc(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:boolean) true: boolean, %BB0, false: boolean, %BB4
// CHECK-NEXT:  %2 = PhiInst (:number) 0: number, %BB0, %11: number, %BB4
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS3: any, empty: any
// CHECK-NEXT:       StoreFrameInst %3: environment, %2: number, [%VS3.i]: number
// CHECK-NEXT:       CondBranchInst %1: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = PhiInst (:number) %2: number, %BB1, %15: number, %BB3
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "push": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %3: environment, %VS3: any, %" 5#"(): functionCode
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %7: any, %9: object
// CHECK-NEXT:  %11 = FAddInst (:number) %6: number, 1: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %11: number, [%VS3.i]: number
// CHECK-NEXT:  %13 = FLessThanInst (:boolean) %11: number, 10: number
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = FAddInst (:number) %2: number, 2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %15: number, [%VS3.i]: number
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %19 = LoadPropertyInst (:any) %18: any, "push": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %3: environment, %VS3: any, %" 6#"(): functionCode
// CHECK-NEXT:  %21 = CallInst (:any) %19: any, empty: any, false: boolean, empty: any, undefined: undefined, %18: any, %20: object
// CHECK-NEXT:  %22 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %23 = CallInst (:any) %22: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %11: number
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [i: number]

// CHECK:function foo_testnc_updatenc(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:number) 1: number, %BB0, %11: number, %BB1
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS4: any, empty: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %1: number, [%VS4.i]: number
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "push": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %2: environment, %VS4: any, %" 7#"(): functionCode
// CHECK-NEXT:  %7 = CallInst (:any) %5: any, empty: any, false: boolean, empty: any, undefined: undefined, %4: any, %6: object
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %1: number
// CHECK-NEXT:  %10 = FAddInst (:number) %1: number, 2: number
// CHECK-NEXT:  %11 = FAddInst (:number) %10: number, 1: number
// CHECK-NEXT:  %12 = FLessThanInst (:boolean) %11: number, 10: number
// CHECK-NEXT:        CondBranchInst %12: boolean, %BB1, %BB2
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
