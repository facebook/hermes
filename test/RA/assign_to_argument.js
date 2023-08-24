/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ra %s | %FileCheckOrRegen %s --match-full-lines

function foo(a) {
  a = a;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:globals = [foo]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg1           	%1 = HBCCreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  $Reg2           	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg3           	%3 = StorePropertyInst %1 : closure, %2 : object, "foo" : string
// CHECK-NEXT:  $Reg3           	%4 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  $Reg4           	%5 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg5           	%6 = StoreStackInst %5 : undefined, %4
// CHECK-NEXT:  $Reg5           	%7 = LoadStackInst %4
// CHECK-NEXT:  $Reg6           	%8 = ReturnInst %7
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(a)#2
// CHECK-NEXT:S{foo#0#1()#2} = [a#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst %S{foo#0#1()#2}
// CHECK-NEXT:  $Reg1           	%1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg2           	%2 = HBCStoreToEnvironmentInst %0, %1, [a#2]
// CHECK-NEXT:  $Reg2           	%3 = HBCLoadFromEnvironmentInst %0, [a#2]
// CHECK-NEXT:  $Reg3           	%4 = HBCStoreToEnvironmentInst %0, %3, [a#2]
// CHECK-NEXT:  $Reg3           	%5 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg4           	%6 = ReturnInst %5 : undefined
// CHECK-NEXT:function_end
