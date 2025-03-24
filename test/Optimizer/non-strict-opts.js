/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -target=HBC -dump-ir -O -fno-inline -non-strict %s | %FileCheckOrRegen --match-full-lines %s
//
// Test the effect of optimizations functions in non-strict mode, since their callsites cannot be analyzed.

function main()  {
  function foo(p1) {
    return "value" + p1;
  }
  foo(2)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %main(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:  %1 = CallInst (:string) %0: object, %foo(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined, 2: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(p1: any): string [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// CHECK-NEXT:  %1 = BinaryAddInst (:string) "value": string, %0: any
// CHECK-NEXT:       ReturnInst %1: string
// CHECK-NEXT:function_end
