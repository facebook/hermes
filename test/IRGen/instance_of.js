/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -dump-ra %s -O

function simple_test0(x, y) {
  return x instanceof y;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [simple_test0]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg1           	%1 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg2           	%2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg3           	%3 = HBCCreateFunctionInst %simple_test0#0#1()#2, %0
// CHECK-NEXT:  $Reg4           	%4 = StorePropertyInst %3 : closure, %1 : object, "simple_test0" : string
// CHECK-NEXT:  $Reg4           	%5 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  $Reg5           	%6 = StoreStackInst %2 : undefined, %5
// CHECK-NEXT:  $Reg5           	%7 = LoadStackInst %5
// CHECK-NEXT:  $Reg6           	%8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function simple_test0#0#1(x, y)#2
// CHECK-NEXT:frame = [x#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst %S{simple_test0#0#1()#2}
// CHECK-NEXT:  $Reg1           	%1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg2           	%2 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg3           	%3 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg4           	%4 = HBCStoreToEnvironmentInst %0, %1, [x#2]
// CHECK-NEXT:  $Reg4           	%5 = HBCStoreToEnvironmentInst %0, %2, [y#2]
// CHECK-NEXT:  $Reg4           	%6 = HBCLoadFromEnvironmentInst %0, [x#2]
// CHECK-NEXT:  $Reg5           	%7 = HBCLoadFromEnvironmentInst %0, [y#2]
// CHECK-NEXT:  $Reg6           	%8 = BinaryOperatorInst 'instanceof', %6, %7
// CHECK-NEXT:  $Reg7           	%9 = ReturnInst %8
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $???           	%10 = ReturnInst %3 : undefined
// CHECK-NEXT:function_end
