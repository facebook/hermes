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
// CHECK-NEXT:  %0 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %0: number, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:number) %2: any, type(number)
// CHECK-NEXT:  %4 = UnaryIncInst (:number) %3: number
// CHECK-NEXT:       StoreFrameInst %4: number, [x]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
