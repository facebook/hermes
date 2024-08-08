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
// CHECK-NEXT:globals = [simple_test0]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1           	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg1           	%1 = HBCCreateFunctionInst %simple_test0#0#1()#2, %0
// CHECK-NEXT:  $Reg2           	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg0           	%3 = StorePropertyInst %1 : closure, %2 : object, "simple_test0" : string
// CHECK-NEXT:  $Reg1           	%4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  $Reg2           	%5 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0           	%6 = StoreStackInst %5 : undefined, %4
// CHECK-NEXT:  $Reg1           	%7 = LoadStackInst %4
// CHECK-NEXT:  $Reg0           	%8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function simple_test0#0#1(x, y)#2
// CHECK-NEXT:S{simple_test0#0#1()#2} = [x#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg2           	%0 = HBCCreateEnvironmentInst %S{simple_test0#0#1()#2}
// CHECK-NEXT:  $Reg3           	%1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg1           	%2 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg0           	%3 = HBCStoreToEnvironmentInst %0, %1, [x#2]
// CHECK-NEXT:  $Reg0           	%4 = HBCStoreToEnvironmentInst %0, %2, [y#2]
// CHECK-NEXT:  $Reg1           	%5 = HBCLoadFromEnvironmentInst %0, [x#2]
// CHECK-NEXT:  $Reg2           	%6 = HBCLoadFromEnvironmentInst %0, [y#2]
// CHECK-NEXT:  $Reg1           	%7 = BinaryOperatorInst 'instanceof', %5, %6
// CHECK-NEXT:  $Reg0           	%8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $???           	%9 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $???           	%10 = ReturnInst %9 : undefined
// CHECK-NEXT:function_end
