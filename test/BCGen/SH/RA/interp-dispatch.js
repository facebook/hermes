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
// CHINT-NEXT:             @0 [empty]	   DeclareGlobalVarInst "bench": string
// CHINT-NEXT:  $loc0      @1 [2...3) 	 = HBCCreateEnvironmentInst (:environment)
// CHINT-NEXT:  $loc1      @2 [3...5) 	 = HBCCreateFunctionInst (:object) %bench(): string|number, $loc0: environment
// CHINT-NEXT:  $loc0      @3 [4...7) 	 = HBCGetGlobalObjectInst (:object)
// CHINT-NEXT:             @4 [empty]	   StorePropertyStrictInst $loc1: object, $loc0: object, "bench": string
// CHINT-NEXT:  $loc1      @5 [6...12) 	 = TryLoadGlobalPropertyInst (:any) $loc0: object, "print": string
// CHINT-NEXT:  $loc0      @6 [7...11) 	 = LoadPropertyInst (:any) $loc0: object, "bench": string
// CHINT-NEXT:  $np0       @7 [8...12) 	 = HBCLoadConstInst (:undefined) undefined: undefined
// CHINT-NEXT:  $np2       @8 [9...11) 	 = HBCLoadConstInst (:number) 4000000: number
// CHINT-NEXT:  $np1       @9 [10...11) 	 = HBCLoadConstInst (:number) 100: number
// CHINT-NEXT:  $loc0      @10 [11...12) 	 = CallInst (:any) $loc0: any, empty: any, empty: any, $np0: undefined, $np0: undefined, $np2: number, $np1: number
// CHINT-NEXT:  $loc0      @11 [12...13) 	 = CallInst (:any) $loc1: any, empty: any, empty: any, $np0: undefined, $np0: undefined, $loc0: any
// CHINT-NEXT:             @12 [empty]	   ReturnInst $loc0: any
// CHINT-NEXT:function_end

// CHINT:function bench(lc: any, fc: any): string|number
// CHINT-NEXT:frame = []
// CHINT-NEXT:%BB0:
// CHINT-NEXT:  $np1       @0 [1...33) 	 = HBCLoadConstInst (:number) 0: number
// CHINT-NEXT:  $np0       @1 [2...33) 	 = HBCLoadConstInst (:number) 1: number
// CHINT-NEXT:  $loc3      @2 [3...33) 	 = LoadParamInst (:any) %fc: any
// CHINT-NEXT:  $loc0      @3 [4...5) 	 = LoadParamInst (:any) %lc: any
// CHINT-NEXT:  $loc2      @4 [5...7) 	 = UnaryDecInst (:number|bigint) $loc0: any
// CHINT-NEXT:  $loc1      @5 [6...11) 	 = MovInst (:number) $np1: number
// CHINT-NEXT:  $loc2      @6 [7...10) 	 = MovInst (:number|bigint) $loc2: number|bigint
// CHINT-NEXT:  $loc0      @7 [8...34) 	 = MovInst (:number) $loc1: number
// CHINT-NEXT:             @8 [empty]	   CmpBrGreaterThanOrEqualInst $loc2: number|bigint, $loc0: number, %BB1, %BB2
// CHINT-NEXT:%BB1:
// CHINT-NEXT:  $loc2      @9 [5...13) [32...33) 	 = PhiInst (:number|bigint) $loc2: number|bigint, %BB0, $loc2: number|bigint, %BB3
// CHINT-NEXT:  $loc1      @10 [6...14) [30...33) 	 = PhiInst (:string|number) $loc1: number, %BB0, $loc1: string|number, %BB3
// CHINT-NEXT:  $loc6      @11 [12...17) 	 = UnaryDecInst (:number|bigint) $loc3: any
// CHINT-NEXT:  $loc2      @12 [5...33) 	 = MovInst (:number|bigint) $loc2: number|bigint
// CHINT-NEXT:  $loc1      @13 [6...28) [30...33) 	 = MovInst (:string|number) $loc1: string|number
// CHINT-NEXT:  $loc5      @14 [15...20) 	 = MovInst (:any) $loc3: any
// CHINT-NEXT:  $loc4      @15 [16...27) 	 = MovInst (:any) $loc5: any
// CHINT-NEXT:  $loc6      @16 [17...19) 	 = MovInst (:number|bigint) $loc6: number|bigint
// CHINT-NEXT:             @17 [empty]	   CmpBrGreaterThanInst $loc6: number|bigint, $np0: number, %BB4, %BB3
// CHINT-NEXT:%BB4:
// CHINT-NEXT:  $loc6      @18 [12...26) 	 = PhiInst (:number|bigint) $loc6: number|bigint, %BB1, $loc6: number|bigint, %BB4
// CHINT-NEXT:  $loc5      @19 [15...21) [23...26) 	 = PhiInst (:any) $loc5: any, %BB1, $loc5: number|bigint, %BB4
// CHINT-NEXT:  $loc7      @20 [21...24) 	 = BinaryMultiplyInst (:number|bigint) $loc5: any, $loc6: number|bigint
// CHINT-NEXT:  $loc6      @21 [22...25) 	 = UnaryDecInst (:number|bigint) $loc6: number|bigint
// CHINT-NEXT:  $loc5      @22 [23...25) 	 = MovInst (:number|bigint) $loc7: number|bigint
// CHINT-NEXT:  $loc4      @23 [24...27) 	 = MovInst (:number|bigint) $loc5: number|bigint
// CHINT-NEXT:  $loc6      @24 [25...26) 	 = MovInst (:number|bigint) $loc6: number|bigint
// CHINT-NEXT:             @25 [empty]	   CmpBrGreaterThanInst $loc6: number|bigint, $np0: number, %BB4, %BB3
// CHINT-NEXT:%BB3:
// CHINT-NEXT:  $loc4      @26 [16...28) 	 = PhiInst (:any) $loc4: any, %BB1, $loc4: number|bigint, %BB4
// CHINT-NEXT:  $loc4      @27 [28...31) 	 = BinaryAddInst (:string|number) $loc1: string|number, $loc4: any
// CHINT-NEXT:  $loc2      @28 [29...32) 	 = UnaryDecInst (:number|bigint) $loc2: number|bigint
// CHINT-NEXT:  $loc1      @29 [30...33) 	 = MovInst (:string|number) $loc4: string|number
// CHINT-NEXT:  $loc0      @30 [31...34) 	 = MovInst (:string|number) $loc1: string|number
// CHINT-NEXT:  $loc2      @31 [32...33) 	 = MovInst (:number|bigint) $loc2: number|bigint
// CHINT-NEXT:             @32 [empty]	   CmpBrGreaterThanOrEqualInst $loc2: number|bigint, $np1: number, %BB1, %BB2
// CHINT-NEXT:%BB2:
// CHINT-NEXT:  $loc0      @33 [8...35) 	 = PhiInst (:string|number) $loc0: number, %BB0, $loc0: string|number, %BB3
// CHINT-NEXT:  $loc0      @34 [8...36) 	 = MovInst (:string|number) $loc0: string|number
// CHINT-NEXT:             @35 [empty]	   ReturnInst $loc0: string|number
// CHINT-NEXT:function_end

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:               DeclareGlobalVarInst "bench": string
// CHECK-NEXT:  $loc0      = HBCCreateEnvironmentInst (:environment)
// CHECK-NEXT:  $loc1      = HBCCreateFunctionInst (:object) %bench(): string|number, $loc0: environment
// CHECK-NEXT:  $loc0      = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:               StorePropertyStrictInst $loc1: object, $loc0: object, "bench": string
// CHECK-NEXT:  $loc1      = TryLoadGlobalPropertyInst (:any) $loc0: object, "print": string
// CHECK-NEXT:  $loc0      = LoadPropertyInst (:any) $loc0: object, "bench": string
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $np2       = HBCLoadConstInst (:number) 4000000: number
// CHECK-NEXT:  $np1       = HBCLoadConstInst (:number) 100: number
// CHECK-NEXT:  $loc0      = CallInst (:any) $loc0: any, empty: any, empty: any, $np0: undefined, $np0: undefined, $np2: number, $np1: number
// CHECK-NEXT:  $loc0      = CallInst (:any) $loc1: any, empty: any, empty: any, $np0: undefined, $np0: undefined, $loc0: any
// CHECK-NEXT:               ReturnInst $loc0: any
// CHECK-NEXT:function_end

// CHECK:function bench(lc: any, fc: any): string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $np1       = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $np0       = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $loc3      = LoadParamInst (:any) %fc: any
// CHECK-NEXT:  $loc0      = LoadParamInst (:any) %lc: any
// CHECK-NEXT:  $loc2      = UnaryDecInst (:number|bigint) $loc0: any
// CHECK-NEXT:  $loc1      = MovInst (:number) $np1: number
// CHECK-NEXT:  $loc2      = MovInst (:number|bigint) $loc2: number|bigint
// CHECK-NEXT:  $loc0      = MovInst (:number) $loc1: number
// CHECK-NEXT:               CmpBrGreaterThanOrEqualInst $loc2: number|bigint, $loc0: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $loc2      = PhiInst (:number|bigint) $loc2: number|bigint, %BB0, $loc2: number|bigint, %BB3
// CHECK-NEXT:  $loc1      = PhiInst (:string|number) $loc1: number, %BB0, $loc1: string|number, %BB3
// CHECK-NEXT:  $loc6      = UnaryDecInst (:number|bigint) $loc3: any
// CHECK-NEXT:  $loc2      = MovInst (:number|bigint) $loc2: number|bigint
// CHECK-NEXT:  $loc1      = MovInst (:string|number) $loc1: string|number
// CHECK-NEXT:  $loc5      = MovInst (:any) $loc3: any
// CHECK-NEXT:  $loc4      = MovInst (:any) $loc5: any
// CHECK-NEXT:  $loc6      = MovInst (:number|bigint) $loc6: number|bigint
// CHECK-NEXT:               CmpBrGreaterThanInst $loc6: number|bigint, $np0: number, %BB4, %BB3
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $loc6      = PhiInst (:number|bigint) $loc6: number|bigint, %BB1, $loc6: number|bigint, %BB4
// CHECK-NEXT:  $loc5      = PhiInst (:any) $loc5: any, %BB1, $loc5: number|bigint, %BB4
// CHECK-NEXT:  $loc7      = BinaryMultiplyInst (:number|bigint) $loc5: any, $loc6: number|bigint
// CHECK-NEXT:  $loc6      = UnaryDecInst (:number|bigint) $loc6: number|bigint
// CHECK-NEXT:  $loc5      = MovInst (:number|bigint) $loc7: number|bigint
// CHECK-NEXT:  $loc4      = MovInst (:number|bigint) $loc5: number|bigint
// CHECK-NEXT:  $loc6      = MovInst (:number|bigint) $loc6: number|bigint
// CHECK-NEXT:               CmpBrGreaterThanInst $loc6: number|bigint, $np0: number, %BB4, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $loc4      = PhiInst (:any) $loc4: any, %BB1, $loc4: number|bigint, %BB4
// CHECK-NEXT:  $loc4      = BinaryAddInst (:string|number) $loc1: string|number, $loc4: any
// CHECK-NEXT:  $loc2      = UnaryDecInst (:number|bigint) $loc2: number|bigint
// CHECK-NEXT:  $loc1      = MovInst (:string|number) $loc4: string|number
// CHECK-NEXT:  $loc0      = MovInst (:string|number) $loc1: string|number
// CHECK-NEXT:  $loc2      = MovInst (:number|bigint) $loc2: number|bigint
// CHECK-NEXT:               CmpBrGreaterThanOrEqualInst $loc2: number|bigint, $np1: number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $loc0      = PhiInst (:string|number) $loc0: number, %BB0, $loc0: string|number, %BB3
// CHECK-NEXT:  $loc0      = MovInst (:string|number) $loc0: string|number
// CHECK-NEXT:               ReturnInst $loc0: string|number
// CHECK-NEXT:function_end
