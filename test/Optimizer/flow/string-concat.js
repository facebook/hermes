/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir -O %s | %FileCheckOrRegen %s --match-full-lines

return function foo(x: string, y: string): string[] {
  return [x + y, x + 'abc', x + y + 'abc' + 'def' + y];
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function foo(x: string, y: string): object [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:string) %x: string
// CHECK-NEXT:  %1 = LoadParamInst (:string) %y: string
// CHECK-NEXT:  %2 = AllocFastArrayInst (:object) 3: number
// CHECK-NEXT:  %3 = StringConcatInst (:string) %0: string, %1: string
// CHECK-NEXT:       FastArrayPushInst %3: string, %2: object
// CHECK-NEXT:  %5 = StringConcatInst (:string) %0: string, "abc": string
// CHECK-NEXT:       FastArrayPushInst %5: string, %2: object
// CHECK-NEXT:  %7 = StringConcatInst (:string) %0: string, %1: string, "abcdef": string, %1: string
// CHECK-NEXT:       FastArrayPushInst %7: string, %2: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end
