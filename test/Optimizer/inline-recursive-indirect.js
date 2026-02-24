/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O4 -Xinline-max-size=100 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Don't inline recursive functions.

'use strict'
(function main() {
  function fact(n) {
    if (n === 0) return 1;
    return n * fact(n-1);
  }

  function f(n) {
    'noinline'
    return fact(n);
  }

  return f(100);
});

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %main(): functionCode
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [fact: object]

// CHECK:function main(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %VS1: any, %fact(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS1.fact]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %VS1: any, %f(): functionCode
// CHECK-NEXT:  %5 = CallInst (:number) %4: object, %f(): functionCode, true: boolean, %1: environment, undefined: undefined, 0: number, 0: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function fact(n: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadParamInst (:number) %n: number
// CHECK-NEXT:  %2 = FEqualInst (:boolean) %1: number, 0: number
// CHECK-NEXT:       CondBranchInst %2: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadFrameInst (:object) %0: environment, [%VS1.fact]: object
// CHECK-NEXT:  %6 = FSubtractInst (:number) %1: number, 1: number
// CHECK-NEXT:  %7 = CallInst (:number) %5: object, %fact(): functionCode, true: boolean, %0: environment, undefined: undefined, 0: number, %6: number
// CHECK-NEXT:  %8 = FMultiplyInst (:number) %1: number, %7: number
// CHECK-NEXT:       ReturnInst %8: number
// CHECK-NEXT:function_end

// CHECK:function f(n: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [%VS1.fact]: object
// CHECK-NEXT:  %2 = CallInst (:number) %1: object, %fact(): functionCode, true: boolean, %0: environment, undefined: undefined, 0: number, 99: number
// CHECK-NEXT:  %3 = FMultiplyInst (:number) 100: number, %2: number
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:function_end
