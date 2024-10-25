/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheckOrRegen --match-full-lines %s

42;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {n0}      %0 = HBCLoadConstInst (:number) 42: number
// CHECK-NEXT:                 ReturnInst {n0} %0: number
// CHECK-NEXT:function_end
