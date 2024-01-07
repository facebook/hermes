/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-ir --test262 %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that ThrowIfEmpty which always throws still has a valid return type.

x + 1;
let x;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ThrowIfInst (:any) empty: empty, type(empty)
// CHECK-NEXT:  %1 = BinaryAddInst (:string|number) %0: any, 1: number
// CHECK-NEXT:       ReturnInst %1: string|number
// CHECK-NEXT:function_end
