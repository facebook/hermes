/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -Xenable-tdz -custom-opt=typeinference -custom-opt=tdzdedup -dump-ir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKOPT %s

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
// CHECK-NEXT:       DeclareGlobalVarInst "check_after_store": string
// CHECK-NEXT:       DeclareGlobalVarInst "check_after_check": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %check_after_store(): any
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "check_after_store": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %check_after_check(): any
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "check_after_check": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function check_after_store(p: any): any
// CHECK-NEXT:frame = [p: any, inner1: any, x: any|empty]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       StoreFrameInst %0: any, [p]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [inner1]: any
// CHECK-NEXT:       StoreFrameInst empty: empty, [x]: any|empty
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %inner1(): any
// CHECK-NEXT:       StoreFrameInst %4: object, [inner1]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any|empty
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function check_after_check(): any
// CHECK-NEXT:frame = [inner2: any, x: any|empty]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [inner2]: any
// CHECK-NEXT:       StoreFrameInst empty: empty, [x]: any|empty
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %inner2(): any
// CHECK-NEXT:       StoreFrameInst %2: object, [inner2]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [x]: any|empty
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function inner1(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any|empty) [x@check_after_store]: any|empty
// CHECK-NEXT:  %1 = ThrowIfEmptyInst (:any) %0: any|empty
// CHECK-NEXT:       StoreFrameInst 10: number, [x@check_after_store]: any|empty
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [p@check_after_store]: any
// CHECK-NEXT:       CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = LoadFrameInst (:any|empty) [x@check_after_store]: any|empty
// CHECK-NEXT:  %6 = ThrowIfEmptyInst (:any) %5: any|empty
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst 0: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function inner2(p: any): any
// CHECK-NEXT:frame = [p: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       StoreFrameInst %0: any, [p]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any|empty) [x@check_after_check]: any|empty
// CHECK-NEXT:  %3 = ThrowIfEmptyInst (:any) %2: any|empty
// CHECK-NEXT:  %4 = UnaryIncInst (:any) %3: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any|empty) [x@check_after_check]: any|empty
// CHECK-NEXT:  %6 = ThrowIfEmptyInst (:any) %5: any|empty
// CHECK-NEXT:       StoreFrameInst %4: any, [x@check_after_check]: any|empty
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [p]: any
// CHECK-NEXT:       CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any|empty) [x@check_after_check]: any|empty
// CHECK-NEXT:  %11 = ThrowIfEmptyInst (:any) %10: any|empty
// CHECK-NEXT:  %12 = UnaryIncInst (:any) %11: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any|empty) [x@check_after_check]: any|empty
// CHECK-NEXT:  %14 = ThrowIfEmptyInst (:any) %13: any|empty
// CHECK-NEXT:        StoreFrameInst %12: any, [x@check_after_check]: any|empty
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadFrameInst (:any|empty) [x@check_after_check]: any|empty
// CHECK-NEXT:  %19 = ThrowIfEmptyInst (:any) %18: any|empty
// CHECK-NEXT:        ReturnInst %19: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        UnreachableInst
// CHECK-NEXT:function_end

// CHKOPT:function global(): undefined
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:       DeclareGlobalVarInst "check_after_store": string
// CHKOPT-NEXT:       DeclareGlobalVarInst "check_after_check": string
// CHKOPT-NEXT:  %2 = CreateFunctionInst (:object) %check_after_store(): any
// CHKOPT-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "check_after_store": string
// CHKOPT-NEXT:  %4 = CreateFunctionInst (:object) %check_after_check(): any
// CHKOPT-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "check_after_check": string
// CHKOPT-NEXT:  %6 = AllocStackInst (:undefined) $?anon_0_ret: any
// CHKOPT-NEXT:       StoreStackInst undefined: undefined, %6: undefined
// CHKOPT-NEXT:  %8 = LoadStackInst (:undefined) %6: undefined
// CHKOPT-NEXT:       ReturnInst %8: undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function check_after_store(p: any): any
// CHKOPT-NEXT:frame = [p: any, inner1: undefined|object, x: empty|undefined|number]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHKOPT-NEXT:       StoreFrameInst %0: any, [p]: any
// CHKOPT-NEXT:       StoreFrameInst undefined: undefined, [inner1]: undefined|object
// CHKOPT-NEXT:       StoreFrameInst empty: empty, [x]: empty|undefined|number
// CHKOPT-NEXT:  %4 = CreateFunctionInst (:object) %inner1(): undefined|number
// CHKOPT-NEXT:       StoreFrameInst %4: object, [inner1]: undefined|object
// CHKOPT-NEXT:       StoreFrameInst undefined: undefined, [x]: empty|undefined|number
// CHKOPT-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHKOPT-NEXT:       ReturnInst %7: any
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:       UnreachableInst
// CHKOPT-NEXT:function_end

// CHKOPT:function check_after_check(): any
// CHKOPT-NEXT:frame = [inner2: undefined|object, x: any|empty]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:       StoreFrameInst undefined: undefined, [inner2]: undefined|object
// CHKOPT-NEXT:       StoreFrameInst empty: empty, [x]: any|empty
// CHKOPT-NEXT:  %2 = CreateFunctionInst (:object) %inner2(): any
// CHKOPT-NEXT:       StoreFrameInst %2: object, [inner2]: undefined|object
// CHKOPT-NEXT:       StoreFrameInst 0: number, [x]: any|empty
// CHKOPT-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "inner": string
// CHKOPT-NEXT:       ReturnInst %5: any
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:       UnreachableInst
// CHKOPT-NEXT:function_end

// CHKOPT:function inner1(): undefined|number
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadFrameInst (:empty|undefined|number) [x@check_after_store]: empty|undefined|number
// CHKOPT-NEXT:  %1 = ThrowIfEmptyInst (:undefined|number) %0: empty|undefined|number
// CHKOPT-NEXT:       StoreFrameInst 10: number, [x@check_after_store]: empty|undefined|number
// CHKOPT-NEXT:  %3 = LoadFrameInst (:any) [p@check_after_store]: any
// CHKOPT-NEXT:       CondBranchInst %3: any, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %5 = LoadFrameInst (:empty|undefined|number) [x@check_after_store]: empty|undefined|number
// CHKOPT-NEXT:  %6 = UnionNarrowTrustedInst (:undefined|number) %5: empty|undefined|number
// CHKOPT-NEXT:       ReturnInst %6: undefined|number
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:       BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:       ReturnInst 0: number
// CHKOPT-NEXT:%BB4:
// CHKOPT-NEXT:        BranchInst %BB3
// CHKOPT-NEXT:%BB5:
// CHKOPT-NEXT:        UnreachableInst
// CHKOPT-NEXT:function_end

// CHKOPT:function inner2(p: any): any
// CHKOPT-NEXT:frame = [p: any]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHKOPT-NEXT:       StoreFrameInst %0: any, [p]: any
// CHKOPT-NEXT:  %2 = LoadFrameInst (:any|empty) [x@check_after_check]: any|empty
// CHKOPT-NEXT:  %3 = ThrowIfEmptyInst (:any) %2: any|empty
// CHKOPT-NEXT:  %4 = UnaryIncInst (:number|bigint) %3: any
// CHKOPT-NEXT:       StoreFrameInst %4: number|bigint, [x@check_after_check]: any|empty
// CHKOPT-NEXT:  %6 = LoadFrameInst (:any) [p]: any
// CHKOPT-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %8 = LoadFrameInst (:any|empty) [x@check_after_check]: any|empty
// CHKOPT-NEXT:  %9 = UnionNarrowTrustedInst (:any) %8: any|empty
// CHKOPT-NEXT:  %10 = UnaryIncInst (:number|bigint) %9: any
// CHKOPT-NEXT:        StoreFrameInst %10: number|bigint, [x@check_after_check]: any|empty
// CHKOPT-NEXT:        BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:        BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  %14 = LoadFrameInst (:any|empty) [x@check_after_check]: any|empty
// CHKOPT-NEXT:  %15 = UnionNarrowTrustedInst (:any) %14: any|empty
// CHKOPT-NEXT:        ReturnInst %15: any
// CHKOPT-NEXT:%BB4:
// CHKOPT-NEXT:        UnreachableInst
// CHKOPT-NEXT:function_end
