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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [sink, foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst
// CHECK-NEXT:  $Reg1           	%1 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg2           	%2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg3           	%3 = HBCCreateFunctionInst %sink(), %0
// CHECK-NEXT:  $Reg4           	%4 = StorePropertyLooseInst %3 : closure, %1 : object, "sink" : string
// CHECK-NEXT:  $Reg4           	%5 = HBCCreateFunctionInst %foo(), %0
// CHECK-NEXT:  $Reg5           	%6 = StorePropertyLooseInst %5 : closure, %1 : object, "foo" : string
// CHECK-NEXT:  $Reg5           	%7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  $Reg6           	%8 = StoreStackInst %2 : undefined, %7
// CHECK-NEXT:  $Reg6           	%9 = LoadStackInst %7
// CHECK-NEXT:  $Reg7           	%10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function sink(x, y, z)
// CHECK-NEXT:frame = [x, y, z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst
// CHECK-NEXT:  $Reg1           	%1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg2           	%2 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg3           	%3 = HBCLoadParamInst 3 : number
// CHECK-NEXT:  $Reg4           	%4 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg5           	%5 = HBCStoreToEnvironmentInst %0, %1, [x]
// CHECK-NEXT:  $Reg5           	%6 = HBCStoreToEnvironmentInst %0, %2, [y]
// CHECK-NEXT:  $Reg5           	%7 = HBCStoreToEnvironmentInst %0, %3, [z]
// CHECK-NEXT:  $Reg5           	%8 = ReturnInst %4 : undefined
// CHECK-NEXT:function_end

// CHECK:function foo(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst
// CHECK-NEXT:  $Reg1           	%1 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg2           	%2 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg3           	%3 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg4           	%4 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg5           	%5 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg6           	%6 = HBCStoreToEnvironmentInst %0, %1, [x]
// CHECK-NEXT:  $Reg6           	%7 = HBCLoadFromEnvironmentInst %0, [x]
// CHECK-NEXT:  $Reg7           	%8 = LoadPropertyInst %7, "sink" : string
// CHECK-NEXT:  $Reg8           	%9 = CallInst %8, %7, %2 : number, %3 : number, %4 : number
// CHECK-NEXT:  $Reg8           	%10 = ReturnInst %5 : undefined
// CHECK-NEXT:function_end
