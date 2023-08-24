/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ra %s | %FileCheckOrRegen %s --match-full-lines

function sink(x,y,z) {}

function foo(x) {
  x.sink(1,2,3)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:globals = [sink, foo]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg1           	%1 = HBCCreateFunctionInst %sink#0#1()#2, %0
// CHECK-NEXT:  $Reg2           	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg3           	%3 = StorePropertyInst %1 : closure, %2 : object, "sink" : string
// CHECK-NEXT:  $Reg3           	%4 = HBCCreateFunctionInst %foo#0#1()#3, %0
// CHECK-NEXT:  $Reg4           	%5 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg5           	%6 = StorePropertyInst %4 : closure, %5 : object, "foo" : string
// CHECK-NEXT:  $Reg5           	%7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  $Reg6           	%8 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg7           	%9 = StoreStackInst %8 : undefined, %7
// CHECK-NEXT:  $Reg7           	%10 = LoadStackInst %7
// CHECK-NEXT:  $Reg8           	%11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function sink#0#1(x, y, z)#2
// CHECK-NEXT:S{sink#0#1()#2} = [x#2, y#2, z#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst %S{sink#0#1()#2}
// CHECK-NEXT:  $Reg1           	%1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg2           	%2 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg3           	%3 = HBCLoadParamInst 3 : number
// CHECK-NEXT:  $Reg4           	%4 = HBCStoreToEnvironmentInst %0, %1, [x#2]
// CHECK-NEXT:  $Reg4           	%5 = HBCStoreToEnvironmentInst %0, %2, [y#2]
// CHECK-NEXT:  $Reg4           	%6 = HBCStoreToEnvironmentInst %0, %3, [z#2]
// CHECK-NEXT:  $Reg4           	%7 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg5           	%8 = ReturnInst %7 : undefined
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(x)#2
// CHECK-NEXT:S{foo#0#1()#2} = [x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst %S{foo#0#1()#2}
// CHECK-NEXT:  $Reg1           	%1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg2           	%2 = HBCStoreToEnvironmentInst %0, %1, [x#2]
// CHECK-NEXT:  $Reg2           	%3 = HBCLoadFromEnvironmentInst %0, [x#2]
// CHECK-NEXT:  $Reg3           	%4 = LoadPropertyInst %3, "sink" : string
// CHECK-NEXT:  $Reg4           	%5 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg5           	%6 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg6           	%7 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg7           	%8 = CallInst %4, undefined : undefined, %3, %5 : number, %6 : number, %7 : number
// CHECK-NEXT:  $Reg7           	%9 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg8           	%10 = ReturnInst %9 : undefined
// CHECK-NEXT:function_end
