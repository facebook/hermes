/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ra %s | %FileCheck %s --match-full-lines

//CHECK-LABEL:function foo(a)
//CHECK-NEXT:frame = [a]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  $Reg0           	%0 = HBCCreateEnvironmentInst
//CHECK-NEXT:  $Reg1           	%1 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  $Reg2           	%2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  $Reg3           	%3 = HBCStoreToEnvironmentInst %0, %1, [a]
//CHECK-NEXT:  $Reg3           	%4 = HBCLoadFromEnvironmentInst %0, [a]
//CHECK-NEXT:  $Reg4           	%5 = HBCStoreToEnvironmentInst %0, %4, [a]
//CHECK-NEXT:  $Reg4           	%6 = ReturnInst %2 : undefined
//CHECK-NEXT:function_end
function foo(a) {
  a = a;
}

