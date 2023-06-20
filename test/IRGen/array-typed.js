/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Test the code generation for some basic array operations.

'use strict';

(function () {
  function foo(x: number[], sink: any) {
    sink(x[0]);
    x[3] = 42;
    for(var i = 0, e = x.length; i < e; i++)
      sink(x[i]);
  }

  return foo;
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): object
// CHECK-NEXT:  %1 = CallInst [njsf] (:object) %0: object, %""(): object, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %2 = ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function ""(): object [allCallsitesKnownInStrictMode,typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %foo(): undefined
// CHECK-NEXT:  %1 = ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function foo(x: any, sink: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %2 = FastArrayLoadInst (:number) %0: any, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: number
// CHECK-NEXT:  %4 = FastArrayStoreInst 42: number, %0: any, 3: number
// CHECK-NEXT:  %5 = FastArrayLengthInst (:number) %0: any
// CHECK-NEXT:  %6 = BinaryLessThanInst (:boolean) 0: number, %5: number
// CHECK-NEXT:  %7 = CondBranchInst %6: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = PhiInst (:number) 0: number, %BB0, %11: number, %BB1
// CHECK-NEXT:  %9 = FastArrayLoadInst (:number) %0: any, %8: number
// CHECK-NEXT:  %10 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: number
// CHECK-NEXT:  %11 = UnaryIncInst (:number) %8: number
// CHECK-NEXT:  %12 = BinaryLessThanInst (:boolean) %11: number, %5: number
// CHECK-NEXT:  %13 = CondBranchInst %12: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
