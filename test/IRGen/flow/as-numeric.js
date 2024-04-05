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

// CHECK:scope %VS0 [exports: any, baz: any]

// CHECK:scope %VS1 [x: any]

// CHECK:function baz(x: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [%VS1.x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:  %6 = UnaryIncInst (:number) %5: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: number, [%VS1.x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
