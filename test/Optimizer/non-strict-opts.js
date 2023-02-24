/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -target=HBC -dump-ir -O -fno-inline -non-strict %s | %FileCheckOrRegen --match-full-lines %s
//
// Ensure that Hermes-specific optimizations (parameter type inference) are performed in non-strict
// mode. We need to disable inlining because it inlines foo() completely and we can't see the
// inference.

function main()  {
  function foo(p1) {
    return "value" + p1;
  }
  foo(2)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %main(): undefined
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "main": string
// CHECK-NEXT:  %3 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %foo(): string
// CHECK-NEXT:  %1 = CallInst (:string) %0: closure, %foo(): string, empty: any, undefined: undefined, 2: number
// CHECK-NEXT:  %2 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(p1: number): string [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryAddInst (:string) "value": string, 2: number
// CHECK-NEXT:  %1 = ReturnInst (:string) %0: string
// CHECK-NEXT:function_end
