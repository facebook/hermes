/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Small functions are inlined into a function with try/catch, but larger ones
// are not (to avoid JIT/native backend deopt from register spilling).

'use strict'

function outer(a, b) {
  // Small: below the try/catch inline size limit, so it gets inlined.
  function small(x) {
    return x + 1;
  }
  // Large: at/above the try/catch inline size limit, so it stays a call.
  function large(x) {
    return x * x + x * x + x * x + x * x;
  }
  try {
    return small(a) + large(b);
  } catch (e) {
    return e;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %outer(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %1: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function outer(a: any, b: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) empty: any, empty: any, %large(): functionCode
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst (:any)
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst %9: string|number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = BinaryAddInst (:string|number) %0: any, 1: number
// CHECK-NEXT:  %8 = CallInst (:number|bigint) %2: object, %large(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number, %1: any
// CHECK-NEXT:  %9 = BinaryAddInst (:string|number) %7: string|number, %8: number|bigint
// CHECK-NEXT:        TryEndInst %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:function large(x: any): number|bigint [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = BinaryMultiplyInst (:number|bigint) %0: any, %0: any
// CHECK-NEXT:  %2 = BinaryMultiplyInst (:number|bigint) %0: any, %0: any
// CHECK-NEXT:  %3 = BinaryAddInst (:number|bigint) %1: number|bigint, %2: number|bigint
// CHECK-NEXT:  %4 = BinaryMultiplyInst (:number|bigint) %0: any, %0: any
// CHECK-NEXT:  %5 = BinaryAddInst (:number|bigint) %3: number|bigint, %4: number|bigint
// CHECK-NEXT:  %6 = BinaryMultiplyInst (:number|bigint) %0: any, %0: any
// CHECK-NEXT:  %7 = BinaryAddInst (:number|bigint) %5: number|bigint, %6: number|bigint
// CHECK-NEXT:       ReturnInst %7: number|bigint
// CHECK-NEXT:function_end
