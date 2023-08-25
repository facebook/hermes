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

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %main(): number
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocFastArrayInst (:object) 3: number
// CHECK-NEXT:  %1 = FastArrayPushInst 1: number, %0: object
// CHECK-NEXT:  %2 = FastArrayPushInst 2: number, %0: object
// CHECK-NEXT:  %3 = FastArrayPushInst 3: number, %0: object
// CHECK-NEXT:  %4 = FastArrayLoadInst (:number) %0: object, 0: number
// CHECK-NEXT:  %5 = BinaryAddInst (:number) %4: number, 1: number
// CHECK-NEXT:  %6 = ReturnInst %5: number
// CHECK-NEXT:function_end
