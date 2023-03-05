/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir -Xenable-tdz %s | %FileCheckOrRegen %s --match-full-lines

// Ensure that ThrowIfEmpty which always throws still has a valid return type.

x + 1;
let x;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ThrowIfEmptyInst (:nonempty) empty: empty
// CHECK-NEXT:  %1 = BinaryAddInst (:string|number) %0: nonempty, 1: number
// CHECK-NEXT:  %2 = ReturnInst %1: string|number
// CHECK-NEXT:function_end
