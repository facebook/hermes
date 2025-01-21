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

// CHECK:function global(): any
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
// CHECK-NEXT:  %23 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %23: any
// CHECK-NEXT:  %25 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:        StorePropertyLooseInst %25: object, globalObject: object, "arr": string
// CHECK-NEXT:  %27 = LoadStackInst (:any) %23: any
// CHECK-NEXT:        ReturnInst %27: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [?anon_0_this: any, ?anon_1_new.target: undefined|object]

// CHECK:scope %VS2 [i: any|empty]

// CHECK:function foo_full(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS1: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS1.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS1.?anon_1_new.target]: undefined|object
// CHECK-NEXT:  %7 = CreateScopeInst (:environment) %VS2: any, %3: environment
// CHECK-NEXT:       StoreFrameInst %7: environment, empty: empty, [%VS2.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %7: environment, 0: number, [%VS2.i]: any|empty
// CHECK-NEXT:  %10 = AllocStackInst (:any|empty) $i: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any|empty) %7: environment, [%VS2.i]: any|empty
// CHECK-NEXT:        StoreStackInst %11: any|empty, %10: any|empty
// CHECK-NEXT:  %13 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:        StoreStackInst true: boolean, %13: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = CreateScopeInst (:environment) %VS2: any, %3: environment
// CHECK-NEXT:  %17 = LoadStackInst (:any|empty) %10: any|empty
// CHECK-NEXT:        StoreFrameInst %16: environment, %17: any|empty, [%VS2.i]: any|empty
// CHECK-NEXT:  %19 = LoadStackInst (:boolean) %13: boolean
// CHECK-NEXT:        CondBranchInst %19: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %21 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %22 = LoadPropertyInst (:any) %21: any, "push": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %16: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %24 = CallInst (:any) %22: any, empty: any, false: boolean, empty: any, undefined: undefined, %21: any, %23: object
// CHECK-NEXT:  %25 = LoadFrameInst (:any|empty) %16: environment, [%VS2.i]: any|empty
// CHECK-NEXT:  %26 = UnionNarrowTrustedInst (:any) %25: any|empty
// CHECK-NEXT:  %27 = UnaryIncInst (:number|bigint) %26: any
// CHECK-NEXT:        StoreFrameInst %16: environment, %27: number|bigint, [%VS2.i]: any|empty
// CHECK-NEXT:  %29 = BinaryLessThanInst (:boolean) %27: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %29: boolean, %BB4, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %31 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %32 = LoadPropertyInst (:any) %31: any, "push": string
// CHECK-NEXT:  %33 = CreateFunctionInst (:object) %16: environment, %""(): functionCode
// CHECK-NEXT:  %34 = CallInst (:any) %32: any, empty: any, false: boolean, empty: any, undefined: undefined, %31: any, %33: object
// CHECK-NEXT:  %35 = LoadFrameInst (:any|empty) %16: environment, [%VS2.i]: any|empty
// CHECK-NEXT:  %36 = UnionNarrowTrustedInst (:any) %35: any|empty
// CHECK-NEXT:  %37 = BinaryAddInst (:any) %36: any, 2: number
// CHECK-NEXT:        StoreFrameInst %16: environment, %37: any, [%VS2.i]: any|empty
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %40 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %41 = LoadPropertyInst (:any) %40: any, "push": string
// CHECK-NEXT:  %42 = CreateFunctionInst (:object) %16: environment, %" 2#"(): functionCode
// CHECK-NEXT:  %43 = CallInst (:any) %41: any, empty: any, false: boolean, empty: any, undefined: undefined, %40: any, %42: object
// CHECK-NEXT:  %44 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %45 = LoadFrameInst (:any|empty) %16: environment, [%VS2.i]: any|empty
// CHECK-NEXT:  %46 = UnionNarrowTrustedInst (:any) %45: any|empty
// CHECK-NEXT:  %47 = CallInst (:any) %44: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %46: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %49 = LoadFrameInst (:any|empty) %16: environment, [%VS2.i]: any|empty
// CHECK-NEXT:        StoreStackInst %49: any|empty, %10: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %13: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [?anon_0_this: any, ?anon_1_new.target: undefined|object]

// CHECK:scope %VS4 [i: any|empty]

// CHECK:function foo_testnc(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS3: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS3.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS3.?anon_1_new.target]: undefined|object
// CHECK-NEXT:  %7 = CreateScopeInst (:environment) %VS4: any, %3: environment
// CHECK-NEXT:       StoreFrameInst %7: environment, empty: empty, [%VS4.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %7: environment, 0: number, [%VS4.i]: any|empty
// CHECK-NEXT:  %10 = AllocStackInst (:any|empty) $i: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any|empty) %7: environment, [%VS4.i]: any|empty
// CHECK-NEXT:        StoreStackInst %11: any|empty, %10: any|empty
// CHECK-NEXT:  %13 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:        StoreStackInst true: boolean, %13: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = CreateScopeInst (:environment) %VS4: any, %3: environment
// CHECK-NEXT:  %17 = LoadStackInst (:any|empty) %10: any|empty
// CHECK-NEXT:        StoreFrameInst %16: environment, %17: any|empty, [%VS4.i]: any|empty
// CHECK-NEXT:  %19 = LoadStackInst (:boolean) %13: boolean
// CHECK-NEXT:        CondBranchInst %19: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %21 = LoadFrameInst (:any|empty) %16: environment, [%VS4.i]: any|empty
// CHECK-NEXT:  %22 = UnionNarrowTrustedInst (:any) %21: any|empty
// CHECK-NEXT:  %23 = UnaryIncInst (:number|bigint) %22: any
// CHECK-NEXT:        StoreFrameInst %16: environment, %23: number|bigint, [%VS4.i]: any|empty
// CHECK-NEXT:  %25 = BinaryLessThanInst (:boolean) %23: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %25: boolean, %BB4, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %27 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %28 = LoadPropertyInst (:any) %27: any, "push": string
// CHECK-NEXT:  %29 = CreateFunctionInst (:object) %16: environment, %" 3#"(): functionCode
// CHECK-NEXT:  %30 = CallInst (:any) %28: any, empty: any, false: boolean, empty: any, undefined: undefined, %27: any, %29: object
// CHECK-NEXT:  %31 = LoadFrameInst (:any|empty) %16: environment, [%VS4.i]: any|empty
// CHECK-NEXT:  %32 = UnionNarrowTrustedInst (:any) %31: any|empty
// CHECK-NEXT:  %33 = BinaryAddInst (:any) %32: any, 2: number
// CHECK-NEXT:        StoreFrameInst %16: environment, %33: any, [%VS4.i]: any|empty
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %36 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %37 = LoadPropertyInst (:any) %36: any, "push": string
// CHECK-NEXT:  %38 = CreateFunctionInst (:object) %16: environment, %" 4#"(): functionCode
// CHECK-NEXT:  %39 = CallInst (:any) %37: any, empty: any, false: boolean, empty: any, undefined: undefined, %36: any, %38: object
// CHECK-NEXT:  %40 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %41 = LoadFrameInst (:any|empty) %16: environment, [%VS4.i]: any|empty
// CHECK-NEXT:  %42 = UnionNarrowTrustedInst (:any) %41: any|empty
// CHECK-NEXT:  %43 = CallInst (:any) %40: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %42: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %45 = LoadFrameInst (:any|empty) %16: environment, [%VS4.i]: any|empty
// CHECK-NEXT:        StoreStackInst %45: any|empty, %10: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %13: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [?anon_0_this: any, ?anon_1_new.target: undefined|object]

// CHECK:scope %VS6 [i: any|empty]

// CHECK:function foo_updatenc(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS5: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS5.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS5.?anon_1_new.target]: undefined|object
// CHECK-NEXT:  %7 = CreateScopeInst (:environment) %VS6: any, %3: environment
// CHECK-NEXT:       StoreFrameInst %7: environment, empty: empty, [%VS6.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %7: environment, 0: number, [%VS6.i]: any|empty
// CHECK-NEXT:  %10 = AllocStackInst (:any|empty) $i: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any|empty) %7: environment, [%VS6.i]: any|empty
// CHECK-NEXT:        StoreStackInst %11: any|empty, %10: any|empty
// CHECK-NEXT:  %13 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:        StoreStackInst true: boolean, %13: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %16 = CreateScopeInst (:environment) %VS6: any, %3: environment
// CHECK-NEXT:  %17 = LoadStackInst (:any|empty) %10: any|empty
// CHECK-NEXT:        StoreFrameInst %16: environment, %17: any|empty, [%VS6.i]: any|empty
// CHECK-NEXT:  %19 = LoadStackInst (:boolean) %13: boolean
// CHECK-NEXT:        CondBranchInst %19: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %21 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %22 = LoadPropertyInst (:any) %21: any, "push": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %16: environment, %" 5#"(): functionCode
// CHECK-NEXT:  %24 = CallInst (:any) %22: any, empty: any, false: boolean, empty: any, undefined: undefined, %21: any, %23: object
// CHECK-NEXT:  %25 = LoadFrameInst (:any|empty) %16: environment, [%VS6.i]: any|empty
// CHECK-NEXT:  %26 = UnionNarrowTrustedInst (:any) %25: any|empty
// CHECK-NEXT:  %27 = UnaryIncInst (:number|bigint) %26: any
// CHECK-NEXT:        StoreFrameInst %16: environment, %27: number|bigint, [%VS6.i]: any|empty
// CHECK-NEXT:  %29 = BinaryLessThanInst (:boolean) %27: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %29: boolean, %BB4, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %31 = LoadFrameInst (:any|empty) %16: environment, [%VS6.i]: any|empty
// CHECK-NEXT:  %32 = UnionNarrowTrustedInst (:any) %31: any|empty
// CHECK-NEXT:  %33 = BinaryAddInst (:any) %32: any, 2: number
// CHECK-NEXT:        StoreFrameInst %16: environment, %33: any, [%VS6.i]: any|empty
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %36 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %37 = LoadPropertyInst (:any) %36: any, "push": string
// CHECK-NEXT:  %38 = CreateFunctionInst (:object) %16: environment, %" 6#"(): functionCode
// CHECK-NEXT:  %39 = CallInst (:any) %37: any, empty: any, false: boolean, empty: any, undefined: undefined, %36: any, %38: object
// CHECK-NEXT:  %40 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %41 = LoadFrameInst (:any|empty) %16: environment, [%VS6.i]: any|empty
// CHECK-NEXT:  %42 = UnionNarrowTrustedInst (:any) %41: any|empty
// CHECK-NEXT:  %43 = CallInst (:any) %40: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %42: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %45 = LoadFrameInst (:any|empty) %16: environment, [%VS6.i]: any|empty
// CHECK-NEXT:        StoreStackInst %45: any|empty, %10: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %13: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [?anon_0_this: any, ?anon_1_new.target: undefined|object, i: any|empty]

// CHECK:scope %VS8 [i: any|empty]

// CHECK:function foo_testnc_updatenc(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS7: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS7.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS7.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, empty: empty, [%VS7.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %3: environment, 0: number, [%VS7.i]: any|empty
// CHECK-NEXT:  %9 = LoadFrameInst (:any|empty) %3: environment, [%VS7.i]: any|empty
// CHECK-NEXT:  %10 = UnionNarrowTrustedInst (:any) %9: any|empty
// CHECK-NEXT:  %11 = UnaryIncInst (:number|bigint) %10: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %11: number|bigint, [%VS7.i]: any|empty
// CHECK-NEXT:  %13 = BinaryLessThanInst (:boolean) %11: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %15 = CreateScopeInst (:environment) %VS8: any, %3: environment
// CHECK-NEXT:        StoreFrameInst %15: environment, empty: empty, [%VS8.i]: any|empty
// CHECK-NEXT:  %17 = LoadFrameInst (:any|empty) %3: environment, [%VS7.i]: any|empty
// CHECK-NEXT:        StoreFrameInst %15: environment, %17: any|empty, [%VS8.i]: any|empty
// CHECK-NEXT:  %19 = LoadPropertyInst (:any) globalObject: object, "arr": string
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %19: any, "push": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %15: environment, %" 7#"(): functionCode
// CHECK-NEXT:  %22 = CallInst (:any) %20: any, empty: any, false: boolean, empty: any, undefined: undefined, %19: any, %21: object
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %24 = LoadFrameInst (:any|empty) %15: environment, [%VS8.i]: any|empty
// CHECK-NEXT:  %25 = UnionNarrowTrustedInst (:any) %24: any|empty
// CHECK-NEXT:  %26 = CallInst (:any) %23: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %25: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %29 = LoadFrameInst (:any|empty) %3: environment, [%VS7.i]: any|empty
// CHECK-NEXT:  %30 = UnionNarrowTrustedInst (:any) %29: any|empty
// CHECK-NEXT:  %31 = UnaryIncInst (:number|bigint) %30: any
// CHECK-NEXT:        StoreFrameInst %3: environment, %31: number|bigint, [%VS7.i]: any|empty
// CHECK-NEXT:  %33 = BinaryLessThanInst (:boolean) %31: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %33: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %35 = LoadFrameInst (:any|empty) %15: environment, [%VS8.i]: any|empty
// CHECK-NEXT:        StoreFrameInst %3: environment, %35: any|empty, [%VS7.i]: any|empty
// CHECK-NEXT:  %37 = LoadFrameInst (:any|empty) %3: environment, [%VS7.i]: any|empty
// CHECK-NEXT:  %38 = UnionNarrowTrustedInst (:any) %37: any|empty
// CHECK-NEXT:  %39 = BinaryAddInst (:any) %38: any, 2: number
// CHECK-NEXT:        StoreFrameInst %3: environment, %39: any, [%VS7.i]: any|empty
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:scope %VS9 [i: any|empty]

// CHECK:function foo_allnc(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS9: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS9.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS9.i]: any|empty
// CHECK-NEXT:  %4 = LoadFrameInst (:any|empty) %1: environment, [%VS9.i]: any|empty
// CHECK-NEXT:  %5 = UnionNarrowTrustedInst (:any) %4: any|empty
// CHECK-NEXT:  %6 = UnaryIncInst (:number|bigint) %5: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number|bigint, [%VS9.i]: any|empty
// CHECK-NEXT:  %8 = BinaryLessThanInst (:boolean) %6: number|bigint, 10: number
// CHECK-NEXT:       CondBranchInst %8: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any|empty) %1: environment, [%VS9.i]: any|empty
// CHECK-NEXT:  %12 = UnionNarrowTrustedInst (:any) %11: any|empty
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %12: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadFrameInst (:any|empty) %1: environment, [%VS9.i]: any|empty
// CHECK-NEXT:  %17 = UnionNarrowTrustedInst (:any) %16: any|empty
// CHECK-NEXT:  %18 = UnaryIncInst (:number|bigint) %17: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: number|bigint, [%VS9.i]: any|empty
// CHECK-NEXT:  %20 = BinaryLessThanInst (:boolean) %18: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %20: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %22 = LoadFrameInst (:any|empty) %1: environment, [%VS9.i]: any|empty
// CHECK-NEXT:  %23 = UnionNarrowTrustedInst (:any) %22: any|empty
// CHECK-NEXT:  %24 = BinaryAddInst (:any) %23: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: any, [%VS9.i]: any|empty
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:scope %VS10 [?anon_0_this: any, ?anon_1_new.target: undefined|object]

// CHECK:scope %VS11 [i: any|empty, x: any|empty]

// CHECK:function foo_init_capture(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %3 = CreateScopeInst (:environment) %VS10: any, %2: environment
// CHECK-NEXT:       StoreFrameInst %3: environment, %1: object, [%VS10.?anon_0_this]: any
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %3: environment, %5: undefined|object, [%VS10.?anon_1_new.target]: undefined|object
// CHECK-NEXT:  %7 = CreateScopeInst (:environment) %VS11: any, %3: environment
// CHECK-NEXT:       StoreFrameInst %7: environment, empty: empty, [%VS11.i]: any|empty
// CHECK-NEXT:       StoreFrameInst %7: environment, empty: empty, [%VS11.x]: any|empty
// CHECK-NEXT:        StoreFrameInst %7: environment, 0: number, [%VS11.i]: any|empty
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %7: environment, %x(): functionCode
// CHECK-NEXT:        StoreFrameInst %7: environment, %11: object, [%VS11.x]: any|empty
// CHECK-NEXT:  %13 = AllocStackInst (:any|empty) $i: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any|empty) %7: environment, [%VS11.i]: any|empty
// CHECK-NEXT:        StoreStackInst %14: any|empty, %13: any|empty
// CHECK-NEXT:  %16 = AllocStackInst (:any|empty) $x: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any|empty) %7: environment, [%VS11.x]: any|empty
// CHECK-NEXT:        StoreStackInst %17: any|empty, %16: any|empty
// CHECK-NEXT:  %19 = AllocStackInst (:boolean) $?anon_2_first: any
// CHECK-NEXT:        StoreStackInst true: boolean, %19: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %22 = CreateScopeInst (:environment) %VS11: any, %3: environment
// CHECK-NEXT:  %23 = LoadStackInst (:any|empty) %13: any|empty
// CHECK-NEXT:        StoreFrameInst %22: environment, %23: any|empty, [%VS11.i]: any|empty
// CHECK-NEXT:  %25 = LoadStackInst (:any|empty) %16: any|empty
// CHECK-NEXT:        StoreFrameInst %22: environment, %25: any|empty, [%VS11.x]: any|empty
// CHECK-NEXT:  %27 = LoadStackInst (:boolean) %19: boolean
// CHECK-NEXT:        CondBranchInst %27: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %29 = LoadFrameInst (:any|empty) %22: environment, [%VS11.i]: any|empty
// CHECK-NEXT:  %30 = UnionNarrowTrustedInst (:any) %29: any|empty
// CHECK-NEXT:  %31 = BinaryLessThanInst (:boolean) %30: any, 3: number
// CHECK-NEXT:        CondBranchInst %31: boolean, %BB4, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %33 = LoadFrameInst (:any|empty) %22: environment, [%VS11.i]: any|empty
// CHECK-NEXT:  %34 = UnionNarrowTrustedInst (:any) %33: any|empty
// CHECK-NEXT:  %35 = UnaryIncInst (:number|bigint) %34: any
// CHECK-NEXT:        StoreFrameInst %22: environment, %35: number|bigint, [%VS11.i]: any|empty
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %38 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %39 = LoadFrameInst (:any|empty) %22: environment, [%VS11.x]: any|empty
// CHECK-NEXT:  %40 = UnionNarrowTrustedInst (:any) %39: any|empty
// CHECK-NEXT:  %41 = CallInst (:any) %40: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %42 = CallInst (:any) %38: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %41: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %44 = LoadFrameInst (:any|empty) %22: environment, [%VS11.i]: any|empty
// CHECK-NEXT:        StoreStackInst %44: any|empty, %13: any|empty
// CHECK-NEXT:  %46 = LoadFrameInst (:any|empty) %22: environment, [%VS11.x]: any|empty
// CHECK-NEXT:        StoreStackInst %46: any|empty, %16: any|empty
// CHECK-NEXT:        StoreStackInst false: boolean, %19: boolean
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS12 [i: any]

// CHECK:function foo_var(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS12: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS12.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS12.i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS12.i]: any
// CHECK-NEXT:  %5 = UnaryIncInst (:number|bigint) %4: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: number|bigint, [%VS12.i]: any
// CHECK-NEXT:  %7 = BinaryLessThanInst (:boolean) %5: number|bigint, 10: number
// CHECK-NEXT:       CondBranchInst %7: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS12.i]: any
// CHECK-NEXT:  %11 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %10: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS12.i]: any
// CHECK-NEXT:  %15 = UnaryIncInst (:number|bigint) %14: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: number|bigint, [%VS12.i]: any
// CHECK-NEXT:  %17 = BinaryLessThanInst (:boolean) %15: number|bigint, 10: number
// CHECK-NEXT:        CondBranchInst %17: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [%VS12.i]: any
// CHECK-NEXT:  %20 = BinaryAddInst (:any) %19: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: any, [%VS12.i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:scope %VS13 []

// CHECK:arrow ""(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS13: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS2.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS14 []

// CHECK:arrow " 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS14: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS2.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS15 []

// CHECK:arrow " 2#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS15: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS2.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS16 []

// CHECK:arrow " 3#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS16: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS4.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS17 []

// CHECK:arrow " 4#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS17: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS4.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS18 []

// CHECK:arrow " 5#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS6: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS18: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS6.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS19 []

// CHECK:arrow " 6#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS6: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS19: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS6.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS20 []

// CHECK:arrow " 7#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS8: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS20: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS8.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS21 []

// CHECK:arrow x(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS11: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS21: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS11.i]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end
