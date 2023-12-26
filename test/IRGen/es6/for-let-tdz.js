/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -Xenable-tdz -Xcustom-opt=simplestackpromotion -dump-ir %s > /dev/null

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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "arr": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_full": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_testnc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_updatenc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_testnc_updatenc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_allnc": string
// CHECK-NEXT:       DeclareGlobalVarInst "foo_var": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %foo_full(): any
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "foo_full": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %foo_testnc(): any
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "foo_testnc": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %foo_updatenc(): any
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "foo_updatenc": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %foo_testnc_updatenc(): any
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "foo_testnc_updatenc": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %foo_allnc(): any
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "foo_allnc": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %foo_var(): any
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "foo_var": string
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:        StorePropertyLooseInst %21: object, globalObject: object, "arr": string
// CHECK-NEXT:  %23 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        ReturnInst %23: any
// CHECK-NEXT:function_end

// CHECK:function foo_full(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any|empty, i#1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:       StoreFrameInst %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst empty: empty, [i]: any|empty
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any|empty
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:       StoreStackInst true: boolean, %7: boolean
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %11 = UnionNarrowTrustedInst (:any) %10: any|empty
// CHECK-NEXT:        StoreFrameInst %11: any, [i#1]: any
// CHECK-NEXT:  %13 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %16 = LoadPropertyInst (:any) %15: any, "push": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %" 1#"(): any
// CHECK-NEXT:  %18 = CallInst (:any) %16: any, empty: any, empty: any, undefined: undefined, %15: any, %17: object
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %20 = UnaryIncInst (:any) %19: any
// CHECK-NEXT:        StoreFrameInst %20: any, [i#1]: any
// CHECK-NEXT:  %22 = BinaryLessThanInst (:any) %20: any, 10: number
// CHECK-NEXT:        CondBranchInst %22: any, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %24 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %25 = LoadPropertyInst (:any) %24: any, "push": string
// CHECK-NEXT:  %26 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:  %27 = CallInst (:any) %25: any, empty: any, empty: any, undefined: undefined, %24: any, %26: object
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 2: number
// CHECK-NEXT:        StoreFrameInst %29: any, [i#1]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %32 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %33 = LoadPropertyInst (:any) %32: any, "push": string
// CHECK-NEXT:  %34 = CreateFunctionInst (:object) %" 2#"(): any
// CHECK-NEXT:  %35 = CallInst (:any) %33: any, empty: any, empty: any, undefined: undefined, %32: any, %34: object
// CHECK-NEXT:  %36 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %37 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %38 = CallInst (:any) %36: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %37: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %40 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:        StoreFrameInst %40: any, [i]: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %7: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo_testnc(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any|empty, i#1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:       StoreFrameInst %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst empty: empty, [i]: any|empty
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any|empty
// CHECK-NEXT:  %7 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:       StoreStackInst true: boolean, %7: boolean
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %11 = UnionNarrowTrustedInst (:any) %10: any|empty
// CHECK-NEXT:        StoreFrameInst %11: any, [i#1]: any
// CHECK-NEXT:  %13 = LoadStackInst (:boolean) %7: boolean
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %16 = UnaryIncInst (:any) %15: any
// CHECK-NEXT:        StoreFrameInst %16: any, [i#1]: any
// CHECK-NEXT:  %18 = BinaryLessThanInst (:any) %16: any, 10: number
// CHECK-NEXT:        CondBranchInst %18: any, %BB4, %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %21 = LoadPropertyInst (:any) %20: any, "push": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %" 3#"(): any
// CHECK-NEXT:  %23 = CallInst (:any) %21: any, empty: any, empty: any, undefined: undefined, %20: any, %22: object
// CHECK-NEXT:  %24 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %25 = BinaryAddInst (:any) %24: any, 2: number
// CHECK-NEXT:        StoreFrameInst %25: any, [i#1]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %28 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %29 = LoadPropertyInst (:any) %28: any, "push": string
// CHECK-NEXT:  %30 = CreateFunctionInst (:object) %" 4#"(): any
// CHECK-NEXT:  %31 = CallInst (:any) %29: any, empty: any, empty: any, undefined: undefined, %28: any, %30: object
// CHECK-NEXT:  %32 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %33 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %34 = CallInst (:any) %32: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %33: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %36 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:        StoreFrameInst %36: any, [i]: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %7: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo_updatenc(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any|empty, i#1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:       StoreFrameInst %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst empty: empty, [i]: any|empty
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any|empty
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %9 = UnionNarrowTrustedInst (:any) %8: any|empty
// CHECK-NEXT:        StoreFrameInst %9: any, [i#1]: any
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %11: any, "push": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %" 5#"(): any
// CHECK-NEXT:  %14 = CallInst (:any) %12: any, empty: any, empty: any, undefined: undefined, %11: any, %13: object
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %16 = UnaryIncInst (:any) %15: any
// CHECK-NEXT:        StoreFrameInst %16: any, [i#1]: any
// CHECK-NEXT:  %18 = BinaryLessThanInst (:any) %16: any, 10: number
// CHECK-NEXT:        CondBranchInst %18: any, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %21 = LoadPropertyInst (:any) %20: any, "push": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %" 6#"(): any
// CHECK-NEXT:  %23 = CallInst (:any) %21: any, empty: any, empty: any, undefined: undefined, %20: any, %22: object
// CHECK-NEXT:  %24 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %25 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %26 = CallInst (:any) %24: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %25: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:        StoreFrameInst %28: any, [i]: any|empty
// CHECK-NEXT:  %30 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %31 = UnionNarrowTrustedInst (:any) %30: any|empty
// CHECK-NEXT:  %32 = BinaryAddInst (:any) %31: any, 2: number
// CHECK-NEXT:        StoreFrameInst %32: any, [i]: any|empty
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo_testnc_updatenc(): any
// CHECK-NEXT:frame = [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any|empty, i#1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:       StoreFrameInst %1: object, [?anon_0_this]: any
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: undefined|object, [?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst empty: empty, [i]: any|empty
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any|empty
// CHECK-NEXT:  %7 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %8 = UnionNarrowTrustedInst (:any) %7: any|empty
// CHECK-NEXT:  %9 = UnaryIncInst (:any) %8: any
// CHECK-NEXT:        StoreFrameInst %9: any, [i]: any|empty
// CHECK-NEXT:  %11 = BinaryLessThanInst (:any) %9: any, 10: number
// CHECK-NEXT:        CondBranchInst %11: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %15 = UnionNarrowTrustedInst (:any) %14: any|empty
// CHECK-NEXT:        StoreFrameInst %15: any, [i#1]: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %19 = LoadPropertyInst (:any) %18: any, "push": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %" 7#"(): any
// CHECK-NEXT:  %21 = CallInst (:any) %19: any, empty: any, empty: any, undefined: undefined, %18: any, %20: object
// CHECK-NEXT:  %22 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:  %24 = CallInst (:any) %22: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %23: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %26 = LoadFrameInst (:any) [i#1]: any
// CHECK-NEXT:        StoreFrameInst %26: any, [i]: any|empty
// CHECK-NEXT:  %28 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %29 = UnionNarrowTrustedInst (:any) %28: any|empty
// CHECK-NEXT:  %30 = BinaryAddInst (:any) %29: any, 2: number
// CHECK-NEXT:        StoreFrameInst %30: any, [i]: any|empty
// CHECK-NEXT:  %32 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %33 = UnionNarrowTrustedInst (:any) %32: any|empty
// CHECK-NEXT:  %34 = UnaryIncInst (:any) %33: any
// CHECK-NEXT:        StoreFrameInst %34: any, [i]: any|empty
// CHECK-NEXT:  %36 = BinaryLessThanInst (:any) %34: any, 10: number
// CHECK-NEXT:        CondBranchInst %36: any, %BB3, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo_allnc(): any
// CHECK-NEXT:frame = [i: any|empty]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst empty: empty, [i]: any|empty
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any|empty
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %3 = UnionNarrowTrustedInst (:any) %2: any|empty
// CHECK-NEXT:  %4 = UnaryIncInst (:any) %3: any
// CHECK-NEXT:       StoreFrameInst %4: any, [i]: any|empty
// CHECK-NEXT:  %6 = BinaryLessThanInst (:any) %4: any, 10: number
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %10 = UnionNarrowTrustedInst (:any) %9: any|empty
// CHECK-NEXT:  %11 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %10: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %15 = UnionNarrowTrustedInst (:any) %14: any|empty
// CHECK-NEXT:  %16 = UnaryIncInst (:any) %15: any
// CHECK-NEXT:        StoreFrameInst %16: any, [i]: any|empty
// CHECK-NEXT:  %18 = BinaryLessThanInst (:any) %16: any, 10: number
// CHECK-NEXT:        CondBranchInst %18: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadFrameInst (:any|empty) [i]: any|empty
// CHECK-NEXT:  %21 = UnionNarrowTrustedInst (:any) %20: any|empty
// CHECK-NEXT:  %22 = BinaryAddInst (:any) %21: any, 2: number
// CHECK-NEXT:        StoreFrameInst %22: any, [i]: any|empty
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:function foo_var(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = UnaryIncInst (:any) %2: any
// CHECK-NEXT:       StoreFrameInst %3: any, [i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:any) %3: any, 10: number
// CHECK-NEXT:       CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %8: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %13 = UnaryIncInst (:any) %12: any
// CHECK-NEXT:        StoreFrameInst %13: any, [i]: any
// CHECK-NEXT:  %15 = BinaryLessThanInst (:any) %13: any, 10: number
// CHECK-NEXT:        CondBranchInst %15: any, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %18 = BinaryAddInst (:any) %17: any, 2: number
// CHECK-NEXT:        StoreFrameInst %18: any, [i]: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:arrow ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [i@foo_full]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:arrow " 1#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [i@foo_full]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:arrow " 2#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [i@foo_full]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:arrow " 3#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [i@foo_testnc]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:arrow " 4#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [i@foo_testnc]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:arrow " 5#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [i@foo_updatenc]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:arrow " 6#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [i@foo_updatenc]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end

// CHECK:arrow " 7#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [i@foo_testnc_updatenc]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:       ReturnInst %1: any
// CHECK-NEXT:function_end
