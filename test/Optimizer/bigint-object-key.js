/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

({ 1_2n: null });

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectLiteralInst (:object) empty: any, 12: number, null: null
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end
