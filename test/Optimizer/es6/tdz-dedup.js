/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xenable-tdz -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -Xenable-tdz -custom-opt=typeinference -custom-opt=tdzdedup -dump-ir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKOPT %s

function check_after_store(p) {
    let x = 10;
    if (p)
        return x;
    return 0;
}

function check_after_check() {
    function inner(p) {
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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "check_after_store": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "check_after_check": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %check_after_store(): any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: closure, globalObject: object, "check_after_store": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %check_after_check(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "check_after_check": string
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %7 = StoreStackInst undefined: undefined, %6: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %6: any
// CHECK-NEXT:  %9 = ReturnInst (:any) %8: any
// CHECK-NEXT:function_end

// CHECK:function check_after_store(p: any): any
// CHECK-NEXT:frame = [p: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [p]: any
// CHECK-NEXT:  %2 = StoreFrameInst empty: empty, [x]: any
// CHECK-NEXT:  %3 = StoreFrameInst 10: number, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [p]: any
// CHECK-NEXT:  %5 = CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = ThrowIfEmptyInst (:any) %6: any
// CHECK-NEXT:  %8 = ReturnInst (:any) %7: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ReturnInst (:any) 0: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function check_after_check(): any
// CHECK-NEXT:frame = [inner: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty: empty, [x]: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %inner(): any
// CHECK-NEXT:  %2 = StoreFrameInst %1: closure, [inner]: any
// CHECK-NEXT:  %3 = StoreFrameInst 0: number, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [inner]: any
// CHECK-NEXT:  %5 = ReturnInst (:any) %4: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function inner(p: any): any
// CHECK-NEXT:frame = [p: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [p]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x@check_after_check]: any
// CHECK-NEXT:  %3 = ThrowIfEmptyInst (:any) %2: any
// CHECK-NEXT:  %4 = UnaryIncInst (:any) %3: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x@check_after_check]: any
// CHECK-NEXT:  %6 = ThrowIfEmptyInst (:any) %5: any
// CHECK-NEXT:  %7 = StoreFrameInst %4: any, [x@check_after_check]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [p]: any
// CHECK-NEXT:  %9 = CondBranchInst %8: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x@check_after_check]: any
// CHECK-NEXT:  %11 = ThrowIfEmptyInst (:any) %10: any
// CHECK-NEXT:  %12 = UnaryIncInst (:any) %11: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [x@check_after_check]: any
// CHECK-NEXT:  %14 = ThrowIfEmptyInst (:any) %13: any
// CHECK-NEXT:  %15 = StoreFrameInst %12: any, [x@check_after_check]: any
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [x@check_after_check]: any
// CHECK-NEXT:  %19 = ThrowIfEmptyInst (:any) %18: any
// CHECK-NEXT:  %20 = ReturnInst (:any) %19: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end

// CHKOPT:function global(): undefined
// CHKOPT-NEXT:frame = []
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = DeclareGlobalVarInst "check_after_store": string
// CHKOPT-NEXT:  %1 = DeclareGlobalVarInst "check_after_check": string
// CHKOPT-NEXT:  %2 = CreateFunctionInst (:closure) %check_after_store(): empty|undefined|number
// CHKOPT-NEXT:  %3 = StorePropertyLooseInst %2: closure, globalObject: object, "check_after_store": string
// CHKOPT-NEXT:  %4 = CreateFunctionInst (:closure) %check_after_check(): undefined|closure
// CHKOPT-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "check_after_check": string
// CHKOPT-NEXT:  %6 = AllocStackInst (:undefined) $?anon_0_ret: any
// CHKOPT-NEXT:  %7 = StoreStackInst undefined: undefined, %6: undefined
// CHKOPT-NEXT:  %8 = LoadStackInst (:undefined) %6: undefined
// CHKOPT-NEXT:  %9 = ReturnInst (:undefined) %8: undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function check_after_store(p: any): empty|undefined|number
// CHKOPT-NEXT:frame = [p: any, x: empty|number]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHKOPT-NEXT:  %1 = StoreFrameInst %0: any, [p]: any
// CHKOPT-NEXT:  %2 = StoreFrameInst empty: empty, [x]: empty|number
// CHKOPT-NEXT:  %3 = StoreFrameInst 10: number, [x]: empty|number
// CHKOPT-NEXT:  %4 = LoadFrameInst (:any) [p]: any
// CHKOPT-NEXT:  %5 = CondBranchInst %4: any, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %6 = LoadFrameInst (:empty|number) [x]: empty|number
// CHKOPT-NEXT:  %7 = ReturnInst (:empty|number) %6: empty|number
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:  %8 = BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  %9 = ReturnInst (:number) 0: number
// CHKOPT-NEXT:%BB4:
// CHKOPT-NEXT:  %10 = BranchInst %BB3
// CHKOPT-NEXT:%BB5:
// CHKOPT-NEXT:  %11 = ReturnInst (:undefined) undefined: undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function check_after_check(): undefined|closure
// CHKOPT-NEXT:frame = [inner: closure, x: any]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = StoreFrameInst empty: empty, [x]: any
// CHKOPT-NEXT:  %1 = CreateFunctionInst (:closure) %inner(): any
// CHKOPT-NEXT:  %2 = StoreFrameInst %1: closure, [inner]: closure
// CHKOPT-NEXT:  %3 = StoreFrameInst 0: number, [x]: any
// CHKOPT-NEXT:  %4 = LoadFrameInst (:closure) [inner]: closure
// CHKOPT-NEXT:  %5 = ReturnInst (:closure) %4: closure
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %6 = ReturnInst (:undefined) undefined: undefined
// CHKOPT-NEXT:function_end

// CHKOPT:function inner(p: any): any
// CHKOPT-NEXT:frame = [p: any]
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHKOPT-NEXT:  %1 = StoreFrameInst %0: any, [p]: any
// CHKOPT-NEXT:  %2 = LoadFrameInst (:any) [x@check_after_check]: any
// CHKOPT-NEXT:  %3 = ThrowIfEmptyInst (:any) %2: any
// CHKOPT-NEXT:  %4 = UnaryIncInst (:number|bigint) %3: any
// CHKOPT-NEXT:  %5 = StoreFrameInst %4: number|bigint, [x@check_after_check]: any
// CHKOPT-NEXT:  %6 = LoadFrameInst (:any) [p]: any
// CHKOPT-NEXT:  %7 = CondBranchInst %6: any, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  %8 = LoadFrameInst (:any) [x@check_after_check]: any
// CHKOPT-NEXT:  %9 = UnaryIncInst (:number|bigint) %8: any
// CHKOPT-NEXT:  %10 = StoreFrameInst %9: number|bigint, [x@check_after_check]: any
// CHKOPT-NEXT:  %11 = BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:  %12 = BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  %13 = LoadFrameInst (:any) [x@check_after_check]: any
// CHKOPT-NEXT:  %14 = ReturnInst (:any) %13: any
// CHKOPT-NEXT:%BB4:
// CHKOPT-NEXT:  %15 = ReturnInst (:undefined) undefined: undefined
// CHKOPT-NEXT:function_end
