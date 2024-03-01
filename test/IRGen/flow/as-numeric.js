/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions=baz -Werror -typed -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// NOTE: this test doesn't test anything yet, since its goal is to demonstrate that
// AsNumericInst narrows its result type based on the type of the input.

function baz(x: number): void {
  x++;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function baz(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %baz(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:  %6 = UnaryIncInst (:number) %5: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number, [x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
