/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ra %s | %FileCheck %s --match-full-lines

function sink(x,y,z) {}

//CHECK-LABEL:function foo(x)
//CHECK-NEXT:frame = [x]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst
//CHECK-NEXT:  $Reg1           	%1 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  $Reg2           	%2 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  $Reg3           	%3 = HBCLoadConstInst 2 : number
//CHECK-NEXT:  $Reg4           	%4 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  $Reg5           	%5 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  $Reg6           	%6 = HBCStoreToEnvironmentInst %0, %1, [x]
//CHECK-NEXT:  $Reg6           	%7 = HBCLoadFromEnvironmentInst %0, [x]
//CHECK-NEXT:  $Reg7           	%8 = LoadPropertyInst %7, "sink" : string
//CHECK-NEXT:  $Reg8           	%9 = CallInst %8, %7, %2 : number, %3 : number, %4 : number
//CHECK-NEXT:  $Reg8           	%10 = ReturnInst %5 : undefined
//CHECK-NEXT:function_end

function foo(x) {
  x.sink(1,2,3)
}
