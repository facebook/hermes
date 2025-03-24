/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir -O %s | %FileCheckOrRegen %s --match-full-lines

function main() {
  let x: number[] = [1, 2, 3];
  return x[0] + 1;
}

// Prevent the function from being optimized out.
sink(main);

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %main(): functionCode
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %2 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocFastArrayInst (:object) 3: number
// CHECK-NEXT:       FastArrayPushInst 1: number, %0: object
// CHECK-NEXT:       FastArrayPushInst 2: number, %0: object
// CHECK-NEXT:       FastArrayPushInst 3: number, %0: object
// CHECK-NEXT:  %4 = FastArrayLoadInst (:number) %0: object, 0: number
// CHECK-NEXT:  %5 = FAddInst (:number) %4: number, 1: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end
