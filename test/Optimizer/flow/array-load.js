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

// CHECK:scope %VS0 [?Array.prototype: object]

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %Array(): functionCode
// CHECK-NEXT:  %2 = AllocTypedNonEnumObjectInst (:object) null: null
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: object, [%VS0.?Array.prototype]: object
// CHECK-NEXT:       StorePropertyStrictInst %2: object, %1: object, "prototype": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %main(): functionCode
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %5: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function Array(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [%VS0.?Array.prototype]: object
// CHECK-NEXT:  %2 = AllocFastArrayInst (:object) 3: number, %1: object
// CHECK-NEXT:       FastArrayPushInst 1: number, %2: object
// CHECK-NEXT:       FastArrayPushInst 2: number, %2: object
// CHECK-NEXT:       FastArrayPushInst 3: number, %2: object
// CHECK-NEXT:  %6 = FastArrayLoadInst (:number) %2: object, 0: number
// CHECK-NEXT:  %7 = FAddInst (:number) %6: number, 1: number
// CHECK-NEXT:       ReturnInst %7: number
// CHECK-NEXT:function_end
