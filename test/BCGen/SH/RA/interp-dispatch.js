/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-ra -dump-register-interval %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHINT %s
// RUN: %shermes -O -dump-ra %s | %FileCheckOrRegen --match-full-lines %s

// Test BB ordering of allocated code.

"use strict";

function bench (lc, fc) {
    var n, fact;
    var res = 0;
    while (--lc >= 0) {
        n = fc;
        fact = n;
        while (--n > 1)
            fact *= n;
        res += fact;
    }
    return res;
}

print(bench(4e6, 100))

// Auto-generated content below. Please do not modify manually.

// CHINT:function global(): any
// CHINT-NEXT:%BB0:
// CHINT-NEXT:            %0            = DeclareGlobalVarInst "bench": string
// CHINT-NEXT:  {loc0}    %1 [2...6)    = HBCGetGlobalObjectInst (:object)
// CHINT-NEXT:  {loc1}    %2 [3...4)    = CreateFunctionInst (:object) empty: any, empty: any, %bench(): functionCode
// CHINT-NEXT:            %3            = StorePropertyStrictInst %2: object, %1: object, "bench": string
// CHINT-NEXT:  {loc1}    %4 [5...11)   = TryLoadGlobalPropertyInst (:any) %1: object, "print": string
// CHINT-NEXT:  {loc0}    %5 [6...10)   = LoadPropertyInst (:any) %1: object, "bench": string
// CHINT-NEXT:  {np2}     %6 [7...10)   = HBCLoadConstInst (:number) 100: number
// CHINT-NEXT:  {np1}     %7 [8...10)   = HBCLoadConstInst (:number) 4000000: number
// CHINT-NEXT:  {np0}     %8 [9...11)   = HBCLoadConstInst (:undefined) undefined: undefined
// CHINT-NEXT:  {loc0}    %9 [10...11)  = CallInst (:any) %5: any, empty: any, false: boolean, empty: any, %8: undefined, %8: undefined, %7: number, %6: number
// CHINT-NEXT:  {loc0}   %10 [11...12)  = CallInst (:any) %4: any, empty: any, false: boolean, empty: any, %8: undefined, %8: undefined, %9: any
// CHINT-NEXT:           %11            = ReturnInst %10: any
// CHINT-NEXT:function_end

// CHINT:function bench(lc: any, fc: any): string|number
// CHINT-NEXT:%BB0:
// CHINT-NEXT:  {loc3}    %0 [1...37)   = LoadParamInst (:any) %fc: any
// CHINT-NEXT:  {loc0}    %1 [2...3)    = LoadParamInst (:any) %lc: any
// CHINT-NEXT:  {loc2}    %2 [3...8)    = UnaryDecInst (:number|bigint) %1: any
// CHINT-NEXT:  {np2}     %3 [4...37)   = HBCLoadConstInst (:number) 0: number
// CHINT-NEXT:  {np0}     %4 [5...10)   = BinaryGreaterThanOrEqualInst (:boolean) %2: number|bigint, %3: number
// CHINT-NEXT:  {np1}     %5 [6...37)   = HBCLoadConstInst (:number) 1: number
// CHINT-NEXT:  {loc1}    %6 [7...11)   = MovInst (:number) %3: number
// CHINT-NEXT:  {loc2}    %7 [8...12)   = MovInst (:number|bigint) %2: number|bigint
// CHINT-NEXT:  {loc0}    %8 [9...38)   = MovInst (:number) %6: number
// CHINT-NEXT:            %9            = CondBranchInst %4: boolean, %BB1, %BB4
// CHINT-NEXT:%BB1:
// CHINT-NEXT:  {loc1}   %10 [7...15) [34...37)  = PhiInst (:string|number) %6: number, %BB0, %33: string|number, %BB3
// CHINT-NEXT:  {loc2}   %11 [3...16) [35...37)  = PhiInst (:number|bigint) %7: number|bigint, %BB0, %34: number|bigint, %BB3
// CHINT-NEXT:  {loc6}   %12 [13...18)  = UnaryDecInst (:number|bigint) %0: any
// CHINT-NEXT:  {np0}    %13 [14...20)  = BinaryGreaterThanInst (:boolean) %12: number|bigint, %5: number
// CHINT-NEXT:  {loc1}   %14 [7...31) [34...37)  = MovInst (:string|number) %10: string|number
// CHINT-NEXT:  {loc2}   %15 [3...37)   = MovInst (:number|bigint) %11: number|bigint
// CHINT-NEXT:  {loc5}   %16 [17...21)  = MovInst (:any) %0: any
// CHINT-NEXT:  {loc6}   %17 [18...22)  = MovInst (:number|bigint) %12: number|bigint
// CHINT-NEXT:  {loc4}   %18 [19...30)  = MovInst (:any) %16: any
// CHINT-NEXT:           %19            = CondBranchInst %13: boolean, %BB2, %BB3
// CHINT-NEXT:%BB2:
// CHINT-NEXT:  {loc5}   %20 [17...23) [26...29)  = PhiInst (:any) %16: any, %BB1, %25: number|bigint, %BB2
// CHINT-NEXT:  {loc6}   %21 [13...29)  = PhiInst (:number|bigint) %17: number|bigint, %BB1, %26: number|bigint, %BB2
// CHINT-NEXT:  {loc7}   %22 [23...28)  = BinaryMultiplyInst (:number|bigint) %20: any, %21: number|bigint
// CHINT-NEXT:  {loc6}   %23 [24...27)  = UnaryDecInst (:number|bigint) %21: number|bigint
// CHINT-NEXT:  {np0}    %24 [25...29)  = BinaryGreaterThanInst (:boolean) %23: number|bigint, %5: number
// CHINT-NEXT:  {loc5}   %25 [26...28)  = MovInst (:number|bigint) %22: number|bigint
// CHINT-NEXT:  {loc6}   %26 [27...28)  = MovInst (:number|bigint) %23: number|bigint
// CHINT-NEXT:  {loc4}   %27 [28...30)  = MovInst (:number|bigint) %25: number|bigint
// CHINT-NEXT:           %28            = CondBranchInst %24: boolean, %BB2, %BB3
// CHINT-NEXT:%BB3:
// CHINT-NEXT:  {loc4}   %29 [19...31)  = PhiInst (:any) %18: any, %BB1, %27: number|bigint, %BB2
// CHINT-NEXT:  {loc4}   %30 [31...36)  = BinaryAddInst (:string|number) %14: string|number, %29: any
// CHINT-NEXT:  {loc2}   %31 [32...35)  = UnaryDecInst (:number|bigint) %15: number|bigint
// CHINT-NEXT:  {np0}    %32 [33...37)  = BinaryGreaterThanOrEqualInst (:boolean) %31: number|bigint, %3: number
// CHINT-NEXT:  {loc1}   %33 [34...37)  = MovInst (:string|number) %30: string|number
// CHINT-NEXT:  {loc2}   %34 [35...37)  = MovInst (:number|bigint) %31: number|bigint
// CHINT-NEXT:  {loc0}   %35 [36...38)  = MovInst (:string|number) %33: string|number
// CHINT-NEXT:           %36            = CondBranchInst %32: boolean, %BB1, %BB4
// CHINT-NEXT:%BB4:
// CHINT-NEXT:  {loc0}   %37 [9...39)   = PhiInst (:string|number) %8: number, %BB0, %35: string|number, %BB3
// CHINT-NEXT:  {loc0}   %38 [9...40)   = MovInst (:string|number) %37: string|number
// CHINT-NEXT:           %39            = ReturnInst %38: string|number
// CHINT-NEXT:function_end

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "bench": string
// CHECK-NEXT:  {loc0}    %1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {loc1}    %2 = CreateFunctionInst (:object) empty: any, empty: any, %bench(): functionCode
// CHECK-NEXT:                 StorePropertyStrictInst {loc1} %2: object, {loc0} %1: object, "bench": string
// CHECK-NEXT:  {loc1}    %4 = TryLoadGlobalPropertyInst (:any) {loc0} %1: object, "print": string
// CHECK-NEXT:  {loc0}    %5 = LoadPropertyInst (:any) {loc0} %1: object, "bench": string
// CHECK-NEXT:  {np2}     %6 = HBCLoadConstInst (:number) 100: number
// CHECK-NEXT:  {np1}     %7 = HBCLoadConstInst (:number) 4000000: number
// CHECK-NEXT:  {np0}     %8 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {loc0}    %9 = CallInst (:any) {loc0} %5: any, empty: any, false: boolean, empty: any, {np0} %8: undefined, {np0} %8: undefined, {np1} %7: number, {np2} %6: number
// CHECK-NEXT:  {loc0}   %10 = CallInst (:any) {loc1} %4: any, empty: any, false: boolean, empty: any, {np0} %8: undefined, {np0} %8: undefined, {loc0} %9: any
// CHECK-NEXT:                 ReturnInst {loc0} %10: any
// CHECK-NEXT:function_end

// CHECK:function bench(lc: any, fc: any): string|number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {loc3}    %0 = LoadParamInst (:any) %fc: any
// CHECK-NEXT:  {loc0}    %1 = LoadParamInst (:any) %lc: any
// CHECK-NEXT:  {loc2}    %2 = UnaryDecInst (:number|bigint) {loc0} %1: any
// CHECK-NEXT:  {np2}     %3 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  {np0}     %4 = BinaryGreaterThanOrEqualInst (:boolean) {loc2} %2: number|bigint, {np2} %3: number
// CHECK-NEXT:  {np1}     %5 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  {loc1}    %6 = MovInst (:number) {np2} %3: number
// CHECK-NEXT:  {loc2}    %7 = MovInst (:number|bigint) {loc2} %2: number|bigint
// CHECK-NEXT:  {loc0}    %8 = MovInst (:number) {loc1} %6: number
// CHECK-NEXT:                 CondBranchInst {np0} %4: boolean, %BB1, %BB4
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {loc1}   %10 = PhiInst (:string|number) {loc1} %6: number, %BB0, {loc1} %33: string|number, %BB3
// CHECK-NEXT:  {loc2}   %11 = PhiInst (:number|bigint) {loc2} %7: number|bigint, %BB0, {loc2} %34: number|bigint, %BB3
// CHECK-NEXT:  {loc6}   %12 = UnaryDecInst (:number|bigint) {loc3} %0: any
// CHECK-NEXT:  {np0}    %13 = BinaryGreaterThanInst (:boolean) {loc6} %12: number|bigint, {np1} %5: number
// CHECK-NEXT:  {loc1}   %14 = MovInst (:string|number) {loc1} %10: string|number
// CHECK-NEXT:  {loc2}   %15 = MovInst (:number|bigint) {loc2} %11: number|bigint
// CHECK-NEXT:  {loc5}   %16 = MovInst (:any) {loc3} %0: any
// CHECK-NEXT:  {loc6}   %17 = MovInst (:number|bigint) {loc6} %12: number|bigint
// CHECK-NEXT:  {loc4}   %18 = MovInst (:any) {loc5} %16: any
// CHECK-NEXT:                 CondBranchInst {np0} %13: boolean, %BB2, %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {loc5}   %20 = PhiInst (:any) {loc5} %16: any, %BB1, {loc5} %25: number|bigint, %BB2
// CHECK-NEXT:  {loc6}   %21 = PhiInst (:number|bigint) {loc6} %17: number|bigint, %BB1, {loc6} %26: number|bigint, %BB2
// CHECK-NEXT:  {loc7}   %22 = BinaryMultiplyInst (:number|bigint) {loc5} %20: any, {loc6} %21: number|bigint
// CHECK-NEXT:  {loc6}   %23 = UnaryDecInst (:number|bigint) {loc6} %21: number|bigint
// CHECK-NEXT:  {np0}    %24 = BinaryGreaterThanInst (:boolean) {loc6} %23: number|bigint, {np1} %5: number
// CHECK-NEXT:  {loc5}   %25 = MovInst (:number|bigint) {loc7} %22: number|bigint
// CHECK-NEXT:  {loc6}   %26 = MovInst (:number|bigint) {loc6} %23: number|bigint
// CHECK-NEXT:  {loc4}   %27 = MovInst (:number|bigint) {loc5} %25: number|bigint
// CHECK-NEXT:                 CondBranchInst {np0} %24: boolean, %BB2, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {loc4}   %29 = PhiInst (:any) {loc4} %18: any, %BB1, {loc4} %27: number|bigint, %BB2
// CHECK-NEXT:  {loc4}   %30 = BinaryAddInst (:string|number) {loc1} %14: string|number, {loc4} %29: any
// CHECK-NEXT:  {loc2}   %31 = UnaryDecInst (:number|bigint) {loc2} %15: number|bigint
// CHECK-NEXT:  {np0}    %32 = BinaryGreaterThanOrEqualInst (:boolean) {loc2} %31: number|bigint, {np2} %3: number
// CHECK-NEXT:  {loc1}   %33 = MovInst (:string|number) {loc4} %30: string|number
// CHECK-NEXT:  {loc2}   %34 = MovInst (:number|bigint) {loc2} %31: number|bigint
// CHECK-NEXT:  {loc0}   %35 = MovInst (:string|number) {loc1} %33: string|number
// CHECK-NEXT:                 CondBranchInst {np0} %32: boolean, %BB1, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  {loc0}   %37 = PhiInst (:string|number) {loc0} %8: number, %BB0, {loc0} %35: string|number, %BB3
// CHECK-NEXT:  {loc0}   %38 = MovInst (:string|number) {loc0} %37: string|number
// CHECK-NEXT:                 ReturnInst {loc0} %38: string|number
// CHECK-NEXT:function_end
