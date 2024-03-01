/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -Xenable-tdz -Xcustom-opt=typeinference,tdzdedup -dump-ir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKOPT %s

function check_after_store(p) {
    function inner1() {
        x = 10;
        if (p)
            return x;
        return 0;
    }
    let x;
    return inner;
}

function check_after_check() {
    function inner2(p) {
        ++x;
        if (p)
            ++x;
        return x;
    }
    let x = 0;
    return inner;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "check_after_store": string
// CHECK-NEXT:       DeclareGlobalVarInst "check_after_check": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %check_after_store(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "check_after_store": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %check_after_check(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "check_after_check": string
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %7: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %7: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function check_after_store(p: any): any
// CHECK-NEXT:frame = [p: any, inner1: any, x: any|empty]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %check_after_store(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [p]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [inner1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [x]: any|empty
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %inner1(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [inner1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: any|empty
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:function check_after_check(): any
// CHECK-NEXT:frame = [inner2: any, x: any|empty]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %check_after_check(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [inner2]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [x]: any|empty
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %inner2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [inner2]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [x]: any|empty
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function inner1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %check_after_store(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %inner1(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %check_after_store(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any|empty) %2: environment, [x@check_after_store]: any|empty
// CHECK-NEXT:  %4 = ThrowIfInst (:any) %3: any|empty, type(empty)
// CHECK-NEXT:       StoreFrameInst %2: environment, 10: number, [x@check_after_store]: any|empty
// CHECK-NEXT:  %6 = ResolveScopeInst (:environment) %check_after_store(): any, %1: environment
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %6: environment, [p@check_after_store]: any
// CHECK-NEXT:       CondBranchInst %7: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ResolveScopeInst (:environment) %check_after_store(): any, %1: environment
// CHECK-NEXT:  %10 = LoadFrameInst (:any|empty) %9: environment, [x@check_after_store]: any|empty
// CHECK-NEXT:  %11 = ThrowIfInst (:any) %10: any|empty, type(empty)
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        ReturnInst 0: number
// CHECK-NEXT:function_end

// CHECK:function inner2(p: any): any
// CHECK-NEXT:frame = [p: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %check_after_check(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %inner2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [p]: any
// CHECK-NEXT:  %4 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:any|empty) %4: environment, [x@check_after_check]: any|empty
// CHECK-NEXT:  %6 = ThrowIfInst (:any) %5: any|empty, type(empty)
// CHECK-NEXT:  %7 = UnaryIncInst (:number|bigint) %6: any
// CHECK-NEXT:  %8 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHECK-NEXT:  %9 = LoadFrameInst (:any|empty) %8: environment, [x@check_after_check]: any|empty
// CHECK-NEXT:  %10 = ThrowIfInst (:any) %9: any|empty, type(empty)
// CHECK-NEXT:        StoreFrameInst %8: environment, %7: number|bigint, [x@check_after_check]: any|empty
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [p]: any
// CHECK-NEXT:        CondBranchInst %12: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %14 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHECK-NEXT:  %15 = LoadFrameInst (:any|empty) %14: environment, [x@check_after_check]: any|empty
// CHECK-NEXT:  %16 = ThrowIfInst (:any) %15: any|empty, type(empty)
// CHECK-NEXT:  %17 = UnaryIncInst (:number|bigint) %16: any
// CHECK-NEXT:  %18 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHECK-NEXT:  %19 = LoadFrameInst (:any|empty) %18: environment, [x@check_after_check]: any|empty
// CHECK-NEXT:  %20 = ThrowIfInst (:any) %19: any|empty, type(empty)
// CHECK-NEXT:        StoreFrameInst %18: environment, %17: number|bigint, [x@check_after_check]: any|empty
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %24 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHECK-NEXT:  %25 = LoadFrameInst (:any|empty) %24: environment, [x@check_after_check]: any|empty
// CHECK-NEXT:  %26 = ThrowIfInst (:any) %25: any|empty, type(empty)
// CHECK-NEXT:        ReturnInst %26: any
// CHECK-NEXT:function_end

// CHKOPT:function global(): undefined
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHKOPT-NEXT:       DeclareGlobalVarInst "check_after_store": string
// CHKOPT-NEXT:       DeclareGlobalVarInst "check_after_check": string
// CHKOPT-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %check_after_store(): functionCode
// CHKOPT-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "check_after_store": string
// CHKOPT-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %check_after_check(): functionCode
// CHKOPT-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "check_after_check": string
// CHKOPT-NEXT:  %7 = AllocStackInst (:undefined) $?anon_0_ret: any
// CHKOPT-NEXT:       StoreStackInst undefined: undefined, %7: undefined
// CHKOPT-NEXT:  %9 = LoadStackInst (:undefined) %7: undefined
// CHKOPT-NEXT:        ReturnInst %9: undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function check_after_store(p: any): any
// CHKOPT-NEXT:frame = [p: any, inner1: undefined|object, x: empty|undefined|number]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHKOPT-NEXT:  %1 = CreateScopeInst (:environment) %check_after_store(): any, %0: environment
// CHKOPT-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHKOPT-NEXT:       StoreFrameInst %1: environment, %2: any, [p]: any
// CHKOPT-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [inner1]: undefined|object
// CHKOPT-NEXT:       StoreFrameInst %1: environment, empty: empty, [x]: empty|undefined|number
// CHKOPT-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %inner1(): functionCode
// CHKOPT-NEXT:       StoreFrameInst %1: environment, %6: object, [inner1]: undefined|object
// CHKOPT-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [x]: empty|undefined|number
// CHKOPT-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHKOPT-NEXT:        ReturnInst %9: any
// CHKOPT-NEXT:function_end

// CHKOPT:function check_after_check(): any
// CHKOPT-NEXT:frame = [inner2: undefined|object, x: empty|number]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHKOPT-NEXT:  %1 = CreateScopeInst (:environment) %check_after_check(): any, %0: environment
// CHKOPT-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [inner2]: undefined|object
// CHKOPT-NEXT:       StoreFrameInst %1: environment, empty: empty, [x]: empty|number
// CHKOPT-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %inner2(): functionCode
// CHKOPT-NEXT:       StoreFrameInst %1: environment, %4: object, [inner2]: undefined|object
// CHKOPT-NEXT:       StoreFrameInst %1: environment, 0: number, [x]: empty|number
// CHKOPT-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHKOPT-NEXT:       ReturnInst %7: any
// CHKOPT-NEXT:function_end

// CHKOPT:function inner1(): undefined|number
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %check_after_store(): any, %parentScope: environment
// CHKOPT-NEXT:  %1 = CreateScopeInst (:environment) %inner1(): any, %0: environment
// CHKOPT-NEXT:  %2 = ResolveScopeInst (:environment) %check_after_store(): any, %1: environment
// CHKOPT-NEXT:  %3 = LoadFrameInst (:empty|undefined|number) %2: environment, [x@check_after_store]: empty|undefined|number
// CHKOPT-NEXT:  %4 = ThrowIfInst (:undefined|number) %3: empty|undefined|number, type(empty)
// CHKOPT-NEXT:       StoreFrameInst %2: environment, 10: number, [x@check_after_store]: empty|undefined|number
// CHKOPT-NEXT:  %6 = ResolveScopeInst (:environment) %check_after_store(): any, %1: environment
// CHKOPT-NEXT:  %7 = LoadFrameInst (:any) %6: environment, [p@check_after_store]: any
// CHKOPT-NEXT:       CondBranchInst %7: any, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %9 = ResolveScopeInst (:environment) %check_after_store(): any, %1: environment
// CHKOPT-NEXT:  %10 = LoadFrameInst (:empty|undefined|number) %9: environment, [x@check_after_store]: empty|undefined|number
// CHKOPT-NEXT:  %11 = UnionNarrowTrustedInst (:undefined|number) %10: empty|undefined|number
// CHKOPT-NEXT:        ReturnInst %11: undefined|number
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:        BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:        ReturnInst 0: number
// CHKOPT-NEXT:function_end

// CHKOPT:function inner2(p: any): number
// CHKOPT-NEXT:frame = [p: any]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %check_after_check(): any, %parentScope: environment
// CHKOPT-NEXT:  %1 = CreateScopeInst (:environment) %inner2(): any, %0: environment
// CHKOPT-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHKOPT-NEXT:       StoreFrameInst %1: environment, %2: any, [p]: any
// CHKOPT-NEXT:  %4 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHKOPT-NEXT:  %5 = LoadFrameInst (:empty|number) %4: environment, [x@check_after_check]: empty|number
// CHKOPT-NEXT:  %6 = ThrowIfInst (:number) %5: empty|number, type(empty)
// CHKOPT-NEXT:  %7 = UnaryIncInst (:number) %6: number
// CHKOPT-NEXT:  %8 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHKOPT-NEXT:       StoreFrameInst %8: environment, %7: number, [x@check_after_check]: empty|number
// CHKOPT-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [p]: any
// CHKOPT-NEXT:        CondBranchInst %10: any, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %12 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHKOPT-NEXT:  %13 = LoadFrameInst (:empty|number) %12: environment, [x@check_after_check]: empty|number
// CHKOPT-NEXT:  %14 = UnionNarrowTrustedInst (:number) %13: empty|number
// CHKOPT-NEXT:  %15 = UnaryIncInst (:number) %14: number
// CHKOPT-NEXT:  %16 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHKOPT-NEXT:        StoreFrameInst %16: environment, %15: number, [x@check_after_check]: empty|number
// CHKOPT-NEXT:        BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:        BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  %20 = ResolveScopeInst (:environment) %check_after_check(): any, %1: environment
// CHKOPT-NEXT:  %21 = LoadFrameInst (:empty|number) %20: environment, [x@check_after_check]: empty|number
// CHKOPT-NEXT:  %22 = UnionNarrowTrustedInst (:number) %21: empty|number
// CHKOPT-NEXT:        ReturnInst %22: number
// CHKOPT-NEXT:function_end
