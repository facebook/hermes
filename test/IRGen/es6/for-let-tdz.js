/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xes6-block-scoping -Xenable-tdz -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -Xes6-block-scoping -Xenable-tdz -Xcustom-opt=simplestackpromotion -dump-ir %s > /dev/null

// Verify code generation for a scoped for-loop.
// Verify IRGen logic for simplifying a scoped for-loop.

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

// The var case, which produces the best possible code.
function foo_var() {
    for(var i = 0; ++i < 10; i += 2) {
        print(i);
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "arr": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_full": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_testnc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_updatenc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_testnc_updatenc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_allnc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_var": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %foo_full(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "foo_full": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %foo_testnc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "foo_testnc": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %foo_updatenc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "foo_updatenc": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %foo_testnc_updatenc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "foo_testnc_updatenc": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %0: environment, %foo_allnc(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "foo_allnc": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %0: environment, %foo_var(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "foo_var": string
// CHECK-NEXT:  %20 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %20: any
// CHECK-NEXT:  %22 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "arr": string
// CHECK-NEXT:  %24 = LoadStackInst (:any) %20: any
// CHECK-NEXT:        ReturnInst %24: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any|empty, i#1: any]

// CHECK:function foo_full(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS1: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS1.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS1.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, empty: empty, [%VS1.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %3: environment, 0: number, [%VS1.i]: any|empty
// CHECK-NEXT:  %9 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:        StoreStackInst true: boolean, %9: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadFrameInst (:any|empty) %3: environment, [%VS1.i]: any|empty
// CHECK-NEXT:  %13 = UnionNarrowTrustedInst (:any) %12: any|empty
// CHECK-NEXT:        StoreFrameInst %3: environment, %13: any, [%VS1.i#1]: any
// CHECK-NEXT:  %15 = LoadStackInst (:boolean) %9: boolean
// CHECK-NEXT:        CondBranchInst %15: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) %17: any, "push": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %3: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %20 = CallInst (:any) %18: any, empty: any, false: boolean, empty: any, undefined: undefined, %17: any, %19: object
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %3: environment, [%VS1.i#1]: any
// CHECK-NEXT:  %22 = UnaryIncInst (:number|bigint) %21: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %22: number|bigint, [%VS1.i#1]: any
// CHECK-NEXT:  %24 = BinaryLessThanInst (:boolean) %22: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %24: boolean, %BB4, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %26 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %27 = LoadPropertyInst (:any) %26: any, "push": string
// CHECK-NEXT:  %28 = CreateFunctionInst (:object) %3: environment, %""(): functionCode
// CHECK-NEXT:  %29 = CallInst (:any) %27: any, empty: any, false: boolean, empty: any, undefined: undefined, %26: any, %28: object
// CHECK-NEXT:  %30 = LoadFrameInst (:any) %3: environment, [%VS1.i#1]: any
// CHECK-NEXT:  %31 = BinaryAddInst (:any) %30: any, 2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %31: any, [%VS1.i#1]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %34 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %35 = LoadPropertyInst (:any) %34: any, "push": string
// CHECK-NEXT:  %36 = CreateFunctionInst (:object) %3: environment, %" 2#"(): functionCode
// CHECK-NEXT:  %37 = CallInst (:any) %35: any, empty: any, false: boolean, empty: any, undefined: undefined, %34: any, %36: object
// CHECK-NEXT:  %38 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %39 = LoadFrameInst (:any) %3: environment, [%VS1.i#1]: any
// CHECK-NEXT:  %40 = CallInst (:any) %38: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %39: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %42 = LoadFrameInst (:any) %3: environment, [%VS1.i#1]: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %42: any, [%VS1.i]: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %9: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any|empty, i#1: any]

// CHECK:function foo_testnc(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS2: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS2.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS2.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, empty: empty, [%VS2.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %3: environment, 0: number, [%VS2.i]: any|empty
// CHECK-NEXT:  %9 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:        StoreStackInst true: boolean, %9: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadFrameInst (:any|empty) %3: environment, [%VS2.i]: any|empty
// CHECK-NEXT:  %13 = UnionNarrowTrustedInst (:any) %12: any|empty
// CHECK-NEXT:        StoreFrameInst %3: environment, %13: any, [%VS2.i#1]: any
// CHECK-NEXT:  %15 = LoadStackInst (:boolean) %9: boolean
// CHECK-NEXT:        CondBranchInst %15: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %3: environment, [%VS2.i#1]: any
// CHECK-NEXT:  %18 = UnaryIncInst (:number|bigint) %17: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %18: number|bigint, [%VS2.i#1]: any
// CHECK-NEXT:  %20 = BinaryLessThanInst (:boolean) %18: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %20: boolean, %BB4, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %22 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %23 = LoadPropertyInst (:any) %22: any, "push": string
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %3: environment, %" 3#"(): functionCode
// CHECK-NEXT:  %25 = CallInst (:any) %23: any, empty: any, false: boolean, empty: any, undefined: undefined, %22: any, %24: object
// CHECK-NEXT:  %26 = LoadFrameInst (:any) %3: environment, [%VS2.i#1]: any
// CHECK-NEXT:  %27 = BinaryAddInst (:any) %26: any, 2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %27: any, [%VS2.i#1]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %30 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %31 = LoadPropertyInst (:any) %30: any, "push": string
// CHECK-NEXT:  %32 = CreateFunctionInst (:object) %3: environment, %" 4#"(): functionCode
// CHECK-NEXT:  %33 = CallInst (:any) %31: any, empty: any, false: boolean, empty: any, undefined: undefined, %30: any, %32: object
// CHECK-NEXT:  %34 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %35 = LoadFrameInst (:any) %3: environment, [%VS2.i#1]: any
// CHECK-NEXT:  %36 = CallInst (:any) %34: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %35: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %38 = LoadFrameInst (:any) %3: environment, [%VS2.i#1]: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %38: any, [%VS2.i]: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %9: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any|empty, i#1: any]

// CHECK:function foo_updatenc(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS3: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS3.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS3.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, empty: empty, [%VS3.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %3: environment, 0: number, [%VS3.i]: any|empty
// CHECK-NEXT:  %9 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:        StoreStackInst true: boolean, %9: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadFrameInst (:any|empty) %3: environment, [%VS3.i]: any|empty
// CHECK-NEXT:  %13 = UnionNarrowTrustedInst (:any) %12: any|empty
// CHECK-NEXT:        StoreFrameInst %3: environment, %13: any, [%VS3.i#1]: any
// CHECK-NEXT:  %15 = LoadStackInst (:boolean) %9: boolean
// CHECK-NEXT:        CondBranchInst %15: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) %17: any, "push": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %3: environment, %" 5#"(): functionCode
// CHECK-NEXT:  %20 = CallInst (:any) %18: any, empty: any, false: boolean, empty: any, undefined: undefined, %17: any, %19: object
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %3: environment, [%VS3.i#1]: any
// CHECK-NEXT:  %22 = UnaryIncInst (:number|bigint) %21: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %22: number|bigint, [%VS3.i#1]: any
// CHECK-NEXT:  %24 = BinaryLessThanInst (:boolean) %22: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %24: boolean, %BB4, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %26 = LoadFrameInst (:any) %3: environment, [%VS3.i#1]: any
// CHECK-NEXT:  %27 = BinaryAddInst (:any) %26: any, 2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %27: any, [%VS3.i#1]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %30 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %31 = LoadPropertyInst (:any) %30: any, "push": string
// CHECK-NEXT:  %32 = CreateFunctionInst (:object) %3: environment, %" 6#"(): functionCode
// CHECK-NEXT:  %33 = CallInst (:any) %31: any, empty: any, false: boolean, empty: any, undefined: undefined, %30: any, %32: object
// CHECK-NEXT:  %34 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %35 = LoadFrameInst (:any) %3: environment, [%VS3.i#1]: any
// CHECK-NEXT:  %36 = CallInst (:any) %34: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %35: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %38 = LoadFrameInst (:any) %3: environment, [%VS3.i#1]: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %38: any, [%VS3.i]: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %9: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any|empty]

// CHECK:scope %VS5 [i: any|empty]

// CHECK:function foo_testnc_updatenc(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS4: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS4.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS4.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, empty: empty, [%VS4.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %3: environment, 0: number, [%VS4.i]: any|empty
// CHECK-NEXT:  %9 = LoadFrameInst (:any|empty) %3: environment, [%VS4.i]: any|empty
// CHECK-NEXT:  %10 = UnionNarrowTrustedInst (:any) %9: any|empty
// CHECK-NEXT:  %11 = UnaryIncInst (:number|bigint) %10: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %11: number|bigint, [%VS4.i]: any|empty
// CHECK-NEXT:  %13 = BinaryLessThanInst (:boolean) %11: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %15 = CreateScopeInst (:environment) %VS5: any, %3: environment
// CHECK-NEXT:        StoreFrameInst %15: environment, empty: empty, [%VS5.i]: any|empty
// CHECK-NEXT:  %17 = LoadFrameInst (:any|empty) %3: environment, [%VS4.i]: any|empty
// CHECK-NEXT:        StoreFrameInst %15: environment, %17: any|empty, [%VS5.i]: any|empty
// CHECK-NEXT:  %19 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %19: any, "push": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %15: environment, %" 7#"(): functionCode
// CHECK-NEXT:  %22 = CallInst (:any) %20: any, empty: any, false: boolean, empty: any, undefined: undefined, %19: any, %21: object
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %24 = LoadFrameInst (:any|empty) %15: environment, [%VS5.i]: any|empty
// CHECK-NEXT:  %25 = UnionNarrowTrustedInst (:any) %24: any|empty
// CHECK-NEXT:  %26 = CallInst (:any) %23: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %25: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %29 = LoadFrameInst (:any|empty) %3: environment, [%VS4.i]: any|empty
// CHECK-NEXT:  %30 = UnionNarrowTrustedInst (:any) %29: any|empty
// CHECK-NEXT:  %31 = UnaryIncInst (:number|bigint) %30: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %31: number|bigint, [%VS4.i]: any|empty
// CHECK-NEXT:  %33 = BinaryLessThanInst (:boolean) %31: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %33: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %35 = LoadFrameInst (:any|empty) %15: environment, [%VS5.i]: any|empty
// CHECK-NEXT:        StoreFrameInst %3: environment, %35: any|empty, [%VS4.i]: any|empty
// CHECK-NEXT:  %37 = LoadFrameInst (:any|empty) %3: environment, [%VS4.i]: any|empty
// CHECK-NEXT:  %38 = UnionNarrowTrustedInst (:any) %37: any|empty
// CHECK-NEXT:  %39 = BinaryAddInst (:any) %38: any, 2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %39: any, [%VS4.i]: any|empty
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [i: any|empty]

// CHECK:function foo_allnc(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS6.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS6.i]: any|empty
// CHECK-NEXT:  %4 = LoadFrameInst (:any|empty) %1: environment, [%VS6.i]: any|empty
// CHECK-NEXT:  %5 = UnionNarrowTrustedInst (:any) %4: any|empty
// CHECK-NEXT:  %6 = UnaryIncInst (:number|bigint) %5: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number|bigint, [%VS6.i]: any|empty
// CHECK-NEXT:  %8 = BinaryLessThanInst (:boolean) %6: number|bigint, 10: number
// CHECK-NEXT:       CondBranchInst %8: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any|empty) %1: environment, [%VS6.i]: any|empty
// CHECK-NEXT:  %12 = UnionNarrowTrustedInst (:any) %11: any|empty
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %12: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadFrameInst (:any|empty) %1: environment, [%VS6.i]: any|empty
// CHECK-NEXT:  %17 = UnionNarrowTrustedInst (:any) %16: any|empty
// CHECK-NEXT:  %18 = UnaryIncInst (:number|bigint) %17: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: number|bigint, [%VS6.i]: any|empty
// CHECK-NEXT:  %20 = BinaryLessThanInst (:boolean) %18: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %20: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = LoadFrameInst (:any|empty) %1: environment, [%VS6.i]: any|empty
// CHECK-NEXT:  %23 = UnionNarrowTrustedInst (:any) %22: any|empty
// CHECK-NEXT:  %24 = BinaryAddInst (:any) %23: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: any, [%VS6.i]: any|empty
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [i: any]

// CHECK:function foo_var(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS7.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS7.i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %5 = UnaryIncInst (:number|bigint) %4: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: number|bigint, [%VS7.i]: any
// CHECK-NEXT:  %7 = BinaryLessThanInst (:boolean) %5: number|bigint, 10: number
// CHECK-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %11 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %10: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %15 = UnaryIncInst (:number|bigint) %14: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: number|bigint, [%VS7.i]: any
// CHECK-NEXT:  %17 = BinaryLessThanInst (:boolean) %15: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %17: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %20 = BinaryAddInst (:any) %19: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: any, [%VS7.i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:scope %VS8 []

// CHECK:arrow ""(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS8: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS1.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS9 []

// CHECK:arrow " 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS9: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS1.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS10 []

// CHECK:arrow " 2#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS10: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS1.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS11 []

// CHECK:arrow " 3#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS11: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS2.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS12 []

// CHECK:arrow " 4#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS12: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS2.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS13 []

// CHECK:arrow " 5#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS13: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS3.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS14 []

// CHECK:arrow " 6#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS14: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS3.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS15 []

// CHECK:arrow " 7#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS5: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS15: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS5.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end
