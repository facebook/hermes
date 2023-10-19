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
// CHINT-NEXT:frame = []
// CHINT-NEXT:%BB0:
// CHINT-NEXT:            %0            = DeclareGlobalVarInst "bench": string
// CHINT-NEXT:  {loc0}    %1 [2...3)    = HBCCreateEnvironmentInst (:environment)
// CHINT-NEXT:  {loc1}    %2 [3...5)    = HBCCreateFunctionInst (:object) %bench(): string|number, %1: environment
// CHINT-NEXT:  {loc0}    %3 [4...7)    = HBCGetGlobalObjectInst (:object)
// CHINT-NEXT:            %4            = StorePropertyStrictInst %2: object, %3: object, "bench": string
// CHINT-NEXT:  {loc1}    %5 [6...12)   = TryLoadGlobalPropertyInst (:any) %3: object, "print": string
// CHINT-NEXT:  {loc0}    %6 [7...11)   = LoadPropertyInst (:any) %3: object, "bench": string
// CHINT-NEXT:  {np0}     %7 [8...12)   = HBCLoadConstInst (:undefined) undefined: undefined
// CHINT-NEXT:  {np2}     %8 [9...11)   = HBCLoadConstInst (:number) 4000000: number
// CHINT-NEXT:  {np1}     %9 [10...11)  = HBCLoadConstInst (:number) 100: number
// CHINT-NEXT:  {loc0}   %10 [11...12)  = CallInst (:any) %6: any, empty: any, empty: any, %7: undefined, %7: undefined, %8: number, %9: number
// CHINT-NEXT:  {loc0}   %11 [12...13)  = CallInst (:any) %5: any, empty: any, empty: any, %7: undefined, %7: undefined, %10: any
// CHINT-NEXT:           %12            = ReturnInst %11: any
// CHINT-NEXT:function_end

// CHINT:function bench(lc: any, fc: any): string|number
// CHINT-NEXT:frame = []
// CHINT-NEXT:%BB0:
// CHINT-NEXT:  {loc3}    %0 [1...33)   = LoadParamInst (:any) %fc: any
// CHINT-NEXT:  {loc0}    %1 [2...3)    = LoadParamInst (:any) %lc: any
// CHINT-NEXT:  {loc2}    %2 [3...7)    = UnaryDecInst (:number|bigint) %1: any
// CHINT-NEXT:  {np1}     %3 [4...33)   = HBCLoadConstInst (:number) 0: number
// CHINT-NEXT:  {np0}     %4 [5...33)   = HBCLoadConstInst (:number) 1: number
// CHINT-NEXT:  {loc1}    %5 [6...11)   = MovInst (:number) %3: number
// CHINT-NEXT:  {loc2}    %6 [7...10)   = MovInst (:number|bigint) %2: number|bigint
// CHINT-NEXT:  {loc0}    %7 [8...34)   = MovInst (:number) %5: number
// CHINT-NEXT:            %8            = CmpBrGreaterThanOrEqualInst %6: number|bigint, %7: number, %BB1, %BB2
// CHINT-NEXT:%BB1:
// CHINT-NEXT:  {loc2}    %9 [3...13) [32...33)  = PhiInst (:number|bigint) %6: number|bigint, %BB0, %31: number|bigint, %BB3
// CHINT-NEXT:  {loc1}   %10 [6...14) [30...33)  = PhiInst (:string|number) %5: number, %BB0, %29: string|number, %BB3
// CHINT-NEXT:  {loc6}   %11 [12...17)  = UnaryDecInst (:number|bigint) %0: any
// CHINT-NEXT:  {loc2}   %12 [3...33)   = MovInst (:number|bigint) %9: number|bigint
// CHINT-NEXT:  {loc1}   %13 [6...28) [30...33)  = MovInst (:string|number) %10: string|number
// CHINT-NEXT:  {loc5}   %14 [15...20)  = MovInst (:any) %0: any
// CHINT-NEXT:  {loc4}   %15 [16...27)  = MovInst (:any) %14: any
// CHINT-NEXT:  {loc6}   %16 [17...19)  = MovInst (:number|bigint) %11: number|bigint
// CHINT-NEXT:           %17            = CmpBrGreaterThanInst %16: number|bigint, %4: number, %BB4, %BB3
// CHINT-NEXT:%BB4:
// CHINT-NEXT:  {loc6}   %18 [12...26)  = PhiInst (:number|bigint) %16: number|bigint, %BB1, %24: number|bigint, %BB4
// CHINT-NEXT:  {loc5}   %19 [15...21) [23...26)  = PhiInst (:any) %14: any, %BB1, %22: number|bigint, %BB4
// CHINT-NEXT:  {loc7}   %20 [21...24)  = BinaryMultiplyInst (:number|bigint) %19: any, %18: number|bigint
// CHINT-NEXT:  {loc6}   %21 [22...25)  = UnaryDecInst (:number|bigint) %18: number|bigint
// CHINT-NEXT:  {loc5}   %22 [23...25)  = MovInst (:number|bigint) %20: number|bigint
// CHINT-NEXT:  {loc4}   %23 [24...27)  = MovInst (:number|bigint) %22: number|bigint
// CHINT-NEXT:  {loc6}   %24 [25...26)  = MovInst (:number|bigint) %21: number|bigint
// CHINT-NEXT:           %25            = CmpBrGreaterThanInst %24: number|bigint, %4: number, %BB4, %BB3
// CHINT-NEXT:%BB3:
// CHINT-NEXT:  {loc4}   %26 [16...28)  = PhiInst (:any) %15: any, %BB1, %23: number|bigint, %BB4
// CHINT-NEXT:  {loc4}   %27 [28...31)  = BinaryAddInst (:string|number) %13: string|number, %26: any
// CHINT-NEXT:  {loc2}   %28 [29...32)  = UnaryDecInst (:number|bigint) %12: number|bigint
// CHINT-NEXT:  {loc1}   %29 [30...33)  = MovInst (:string|number) %27: string|number
// CHINT-NEXT:  {loc0}   %30 [31...34)  = MovInst (:string|number) %29: string|number
// CHINT-NEXT:  {loc2}   %31 [32...33)  = MovInst (:number|bigint) %28: number|bigint
// CHINT-NEXT:           %32            = CmpBrGreaterThanOrEqualInst %31: number|bigint, %3: number, %BB1, %BB2
// CHINT-NEXT:%BB2:
// CHINT-NEXT:  {loc0}   %33 [8...35)   = PhiInst (:string|number) %7: number, %BB0, %30: string|number, %BB3
// CHINT-NEXT:  {loc0}   %34 [8...36)   = MovInst (:string|number) %33: string|number
// CHINT-NEXT:           %35            = ReturnInst %34: string|number
// CHINT-NEXT:function_end

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "bench": string
// CHECK-NEXT:  {loc0}    %1 = HBCCreateEnvironmentInst (:environment)
// CHECK-NEXT:  {loc1}    %2 = HBCCreateFunctionInst (:object) %bench(): string|number, {loc0} %1: environment
// CHECK-NEXT:  {loc0}    %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyStrictInst {loc1} %2: object, {loc0} %3: object, "bench": string
// CHECK-NEXT:  {loc1}    %5 = TryLoadGlobalPropertyInst (:any) {loc0} %3: object, "print": string
// CHECK-NEXT:  {loc0}    %6 = LoadPropertyInst (:any) {loc0} %3: object, "bench": string
// CHECK-NEXT:  {np0}     %7 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {np2}     %8 = HBCLoadConstInst (:number) 4000000: number
// CHECK-NEXT:  {np1}     %9 = HBCLoadConstInst (:number) 100: number
// CHECK-NEXT:  {loc0}   %10 = CallInst (:any) {loc0} %6: any, empty: any, empty: any, {np0} %7: undefined, {np0} %7: undefined, {np2} %8: number, {np1} %9: number
// CHECK-NEXT:  {loc0}   %11 = CallInst (:any) {loc1} %5: any, empty: any, empty: any, {np0} %7: undefined, {np0} %7: undefined, {loc0} %10: any
// CHECK-NEXT:                 ReturnInst {loc0} %11: any
// CHECK-NEXT:function_end

// CHECK:function bench(lc: any, fc: any): string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {loc3}    %0 = LoadParamInst (:any) %fc: any
// CHECK-NEXT:  {loc0}    %1 = LoadParamInst (:any) %lc: any
// CHECK-NEXT:  {loc2}    %2 = UnaryDecInst (:number|bigint) {loc0} %1: any
// CHECK-NEXT:  {np1}     %3 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  {np0}     %4 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  {loc1}    %5 = MovInst (:number) {np1} %3: number
// CHECK-NEXT:  {loc2}    %6 = MovInst (:number|bigint) {loc2} %2: number|bigint
// CHECK-NEXT:  {loc0}    %7 = MovInst (:number) {loc1} %5: number
// CHECK-NEXT:                 CmpBrGreaterThanOrEqualInst {loc2} %6: number|bigint, {loc0} %7: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {loc2}    %9 = PhiInst (:number|bigint) {loc2} %6: number|bigint, %BB0, {loc2} %31: number|bigint, %BB3
// CHECK-NEXT:  {loc1}   %10 = PhiInst (:string|number) {loc1} %5: number, %BB0, {loc1} %29: string|number, %BB3
// CHECK-NEXT:  {loc6}   %11 = UnaryDecInst (:number|bigint) {loc3} %0: any
// CHECK-NEXT:  {loc2}   %12 = MovInst (:number|bigint) {loc2} %9: number|bigint
// CHECK-NEXT:  {loc1}   %13 = MovInst (:string|number) {loc1} %10: string|number
// CHECK-NEXT:  {loc5}   %14 = MovInst (:any) {loc3} %0: any
// CHECK-NEXT:  {loc4}   %15 = MovInst (:any) {loc5} %14: any
// CHECK-NEXT:  {loc6}   %16 = MovInst (:number|bigint) {loc6} %11: number|bigint
// CHECK-NEXT:                 CmpBrGreaterThanInst {loc6} %16: number|bigint, {np0} %4: number, %BB4, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  {loc6}   %18 = PhiInst (:number|bigint) {loc6} %16: number|bigint, %BB1, {loc6} %24: number|bigint, %BB4
// CHECK-NEXT:  {loc5}   %19 = PhiInst (:any) {loc5} %14: any, %BB1, {loc5} %22: number|bigint, %BB4
// CHECK-NEXT:  {loc7}   %20 = BinaryMultiplyInst (:number|bigint) {loc5} %19: any, {loc6} %18: number|bigint
// CHECK-NEXT:  {loc6}   %21 = UnaryDecInst (:number|bigint) {loc6} %18: number|bigint
// CHECK-NEXT:  {loc5}   %22 = MovInst (:number|bigint) {loc7} %20: number|bigint
// CHECK-NEXT:  {loc4}   %23 = MovInst (:number|bigint) {loc5} %22: number|bigint
// CHECK-NEXT:  {loc6}   %24 = MovInst (:number|bigint) {loc6} %21: number|bigint
// CHECK-NEXT:                 CmpBrGreaterThanInst {loc6} %24: number|bigint, {np0} %4: number, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {loc4}   %26 = PhiInst (:any) {loc4} %15: any, %BB1, {loc4} %23: number|bigint, %BB4
// CHECK-NEXT:  {loc4}   %27 = BinaryAddInst (:string|number) {loc1} %13: string|number, {loc4} %26: any
// CHECK-NEXT:  {loc2}   %28 = UnaryDecInst (:number|bigint) {loc2} %12: number|bigint
// CHECK-NEXT:  {loc1}   %29 = MovInst (:string|number) {loc4} %27: string|number
// CHECK-NEXT:  {loc0}   %30 = MovInst (:string|number) {loc1} %29: string|number
// CHECK-NEXT:  {loc2}   %31 = MovInst (:number|bigint) {loc2} %28: number|bigint
// CHECK-NEXT:                 CmpBrGreaterThanOrEqualInst {loc2} %31: number|bigint, {np1} %3: number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {loc0}   %33 = PhiInst (:string|number) {loc0} %7: number, %BB0, {loc0} %30: string|number, %BB3
// CHECK-NEXT:  {loc0}   %34 = MovInst (:string|number) {loc0} %33: string|number
// CHECK-NEXT:                 ReturnInst {loc0} %34: string|number
// CHECK-NEXT:function_end
