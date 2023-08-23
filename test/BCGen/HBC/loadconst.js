/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheckOrRegen --match-full-lines %s

42;
Array != undefined;
null;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : null
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg1 @1 [2...4)	%1 = TryLoadGlobalPropertyInst %0 : object, "Array" : string
// CHECK-NEXT:  $Reg0 @2 [3...5)	%2 = HBCLoadConstInst null : null
// CHECK-NEXT:  $Reg1 @3 [empty]	%3 = BinaryOperatorInst '!=', %1, %2 : null
// CHECK-NEXT:  $Reg0 @4 [empty]	%4 = ReturnInst %2 : null
// CHECK-NEXT:function_end
