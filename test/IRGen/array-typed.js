/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Test the code generation for some basic array operations.

function foo(x: number[], sink: any): void {
sink(x[0]);
sink(x.push(3.14));
x[3] = 42;
for(var i = 0, e = x.length; i < e; i++)
  sink(x[i]);
}

return foo;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): object
// CHECK-NEXT:  %1 = CallInst [njsf] (:object) %0: object, %""(): object, empty: any, undefined: undefined, 0: number, 0: number
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function ""(exports: number): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %foo(): undefined
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function foo(x: object, sink: any): undefined [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %x: object
// CHECK-NEXT:  %1 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %2 = FastArrayLoadInst (:number) %0: object, 0: number
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: number
// CHECK-NEXT:       FastArrayPushInst 3.14: number, %0: object
// CHECK-NEXT:  %5 = FastArrayLengthInst (:number) %0: object
// CHECK-NEXT:  %6 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: number
// CHECK-NEXT:       FastArrayStoreInst 42: number, %0: object, 3: number
// CHECK-NEXT:  %8 = FastArrayLengthInst (:number) %0: object
// CHECK-NEXT:  %9 = FLessThanInst (:boolean) 0: number, %8: number
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = PhiInst (:number) 0: number, %BB0, %14: number, %BB1
// CHECK-NEXT:  %12 = FastArrayLoadInst (:number) %0: object, %11: number
// CHECK-NEXT:  %13 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %12: number
// CHECK-NEXT:  %14 = FAddInst (:number) %11: number, 1: number
// CHECK-NEXT:  %15 = FLessThanInst (:boolean) %14: number, %8: number
// CHECK-NEXT:        CondBranchInst %15: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
