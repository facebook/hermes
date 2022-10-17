/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheckOrRegen --match-full-lines %s

42;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCLoadConstInst 42 : number
// CHECK-NEXT:  $Reg0 @1 [empty]	%1 = ReturnInst %0 : number
// CHECK-NEXT:function_end
