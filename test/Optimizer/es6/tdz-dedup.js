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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
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

// CHECK:scope %VS1 [p: any, inner1: any, x: any|empty]

// CHECK:function check_after_store(p: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.p]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS1.x]: any|empty
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %inner1(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS1.inner1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.x]: any|empty
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [inner2: any, x: any|empty]

// CHECK:function check_after_check(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS2.x]: any|empty
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %inner2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [%VS2.inner2]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS2.x]: any|empty
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function inner1(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) %0: environment, [%VS1.x]: any|empty
// CHECK-NEXT:  %3 = ThrowIfInst (:any) %2: any|empty, type(empty)
// CHECK-NEXT:       StoreFrameInst %0: environment, 10: number, [%VS1.x]: any|empty
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %0: environment, [%VS1.p]: any
// CHECK-NEXT:       CondBranchInst %5: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadFrameInst (:any|empty) %0: environment, [%VS1.x]: any|empty
// CHECK-NEXT:  %8 = ThrowIfInst (:any) %7: any|empty, type(empty)
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        ReturnInst 0: number
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [p: any]

// CHECK:function inner2(p: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS4.p]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any|empty) %0: environment, [%VS2.x]: any|empty
// CHECK-NEXT:  %5 = ThrowIfInst (:any) %4: any|empty, type(empty)
// CHECK-NEXT:  %6 = UnaryIncInst (:number|bigint) %5: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any|empty) %0: environment, [%VS2.x]: any|empty
// CHECK-NEXT:  %8 = ThrowIfInst (:any) %7: any|empty, type(empty)
// CHECK-NEXT:       StoreFrameInst %0: environment, %6: number|bigint, [%VS2.x]: any|empty
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS4.p]: any
// CHECK-NEXT:        CondBranchInst %10: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadFrameInst (:any|empty) %0: environment, [%VS2.x]: any|empty
// CHECK-NEXT:  %13 = ThrowIfInst (:any) %12: any|empty, type(empty)
// CHECK-NEXT:  %14 = UnaryIncInst (:number|bigint) %13: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any|empty) %0: environment, [%VS2.x]: any|empty
// CHECK-NEXT:  %16 = ThrowIfInst (:any) %15: any|empty, type(empty)
// CHECK-NEXT:        StoreFrameInst %0: environment, %14: number|bigint, [%VS2.x]: any|empty
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = LoadFrameInst (:any|empty) %0: environment, [%VS2.x]: any|empty
// CHECK-NEXT:  %21 = ThrowIfInst (:any) %20: any|empty, type(empty)
// CHECK-NEXT:        ReturnInst %21: any
// CHECK-NEXT:function_end

// CHKOPT:scope %VS0 []

// CHKOPT:function global(): undefined
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
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

// CHKOPT:scope %VS1 [p: any, inner1: object, x: empty|undefined|number]

// CHKOPT:function check_after_store(p: any): any
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHKOPT-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHKOPT-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHKOPT-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.p]: any
// CHKOPT-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS1.x]: empty|undefined|number
// CHKOPT-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %inner1(): functionCode
// CHKOPT-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS1.inner1]: object
// CHKOPT-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.x]: empty|undefined|number
// CHKOPT-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHKOPT-NEXT:       ReturnInst %8: any
// CHKOPT-NEXT:function_end

// CHKOPT:scope %VS2 [inner2: object, x: empty|number]

// CHKOPT:function check_after_check(): any
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHKOPT-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHKOPT-NEXT:       StoreFrameInst %1: environment, empty: empty, [%VS2.x]: empty|number
// CHKOPT-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %inner2(): functionCode
// CHKOPT-NEXT:       StoreFrameInst %1: environment, %3: object, [%VS2.inner2]: object
// CHKOPT-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS2.x]: empty|number
// CHKOPT-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHKOPT-NEXT:       ReturnInst %6: any
// CHKOPT-NEXT:function_end

// CHKOPT:scope %VS3 []

// CHKOPT:function inner1(): undefined|number
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHKOPT-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHKOPT-NEXT:  %2 = LoadFrameInst (:empty|undefined|number) %0: environment, [%VS1.x]: empty|undefined|number
// CHKOPT-NEXT:  %3 = ThrowIfInst (:undefined|number) %2: empty|undefined|number, type(empty)
// CHKOPT-NEXT:       StoreFrameInst %0: environment, 10: number, [%VS1.x]: empty|undefined|number
// CHKOPT-NEXT:  %5 = LoadFrameInst (:any) %0: environment, [%VS1.p]: any
// CHKOPT-NEXT:       CondBranchInst %5: any, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %7 = LoadFrameInst (:empty|undefined|number) %0: environment, [%VS1.x]: empty|undefined|number
// CHKOPT-NEXT:  %8 = UnionNarrowTrustedInst (:undefined|number) %7: empty|undefined|number
// CHKOPT-NEXT:       ReturnInst %8: undefined|number
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:        BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:        ReturnInst 0: number
// CHKOPT-NEXT:function_end

// CHKOPT:scope %VS4 [p: any]

// CHKOPT:function inner2(p: any): number
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHKOPT-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHKOPT-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHKOPT-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS4.p]: any
// CHKOPT-NEXT:  %4 = LoadFrameInst (:empty|number) %0: environment, [%VS2.x]: empty|number
// CHKOPT-NEXT:  %5 = ThrowIfInst (:number) %4: empty|number, type(empty)
// CHKOPT-NEXT:  %6 = UnaryIncInst (:number) %5: number
// CHKOPT-NEXT:       StoreFrameInst %0: environment, %6: number, [%VS2.x]: empty|number
// CHKOPT-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS4.p]: any
// CHKOPT-NEXT:       CondBranchInst %8: any, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %10 = LoadFrameInst (:empty|number) %0: environment, [%VS2.x]: empty|number
// CHKOPT-NEXT:  %11 = UnionNarrowTrustedInst (:number) %10: empty|number
// CHKOPT-NEXT:  %12 = UnaryIncInst (:number) %11: number
// CHKOPT-NEXT:        StoreFrameInst %0: environment, %12: number, [%VS2.x]: empty|number
// CHKOPT-NEXT:        BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:        BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  %16 = LoadFrameInst (:empty|number) %0: environment, [%VS2.x]: empty|number
// CHKOPT-NEXT:  %17 = UnionNarrowTrustedInst (:number) %16: empty|number
// CHKOPT-NEXT:        ReturnInst %17: number
// CHKOPT-NEXT:function_end
