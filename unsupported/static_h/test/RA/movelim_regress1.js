/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ra %s | %FileCheck --match-full-lines %s
// Mov elimination incorrectly eliminated a mov in this case.

function fib(n) {
  var f0 = 0, f1 = 1;
  for (; n > 0; n = n -1) {
	var f2 = f0 + f1;
	f0 = f1; f1 = f2;
  }
  return f0;
}

//CHECK-LABEL:function fib(n) : string|number|bigint
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %1 = HBCLoadConstInst 0 : number
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  {{.*}}  %3 = MovInst %1 : number
//CHECK-NEXT:  {{.*}}  %4 = MovInst %2 : number
//CHECK-NEXT:  {{.*}}  %5 = MovInst %0
//CHECK-NEXT:  {{.*}}  %6 = MovInst %3 : number
//CHECK-NEXT:  {{.*}}  %7 = CompareBranchInst '>', %5, %6 : number, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %8 = PhiInst %3 : number, %BB0, %14 : string|number|bigint, %BB1
//CHECK-NEXT:  {{.*}}  %9 = PhiInst %4 : number, %BB0, %15 : string|number|bigint, %BB1
//CHECK-NEXT:  {{.*}}  %10 = PhiInst %5, %BB0, %17 : number, %BB1
//CHECK-NEXT:  {{.*}}  %11 = BinaryOperatorInst '+', %8 : string|number|bigint, %9 : string|number|bigint
//CHECK-NEXT:  {{.*}}  %12 = BinaryOperatorInst '-', %10, %2 : number
//CHECK-NEXT:  {{.*}}  %13 = MovInst %9 : string|number|bigint
//CHECK-NEXT:  {{.*}}  %14 = MovInst %13 : string|number|bigint
//CHECK-NEXT:  {{.*}}  %15 = MovInst %11 : string|number|bigint
//CHECK-NEXT:  {{.*}}  %16 = MovInst %14 : string|number|bigint
//CHECK-NEXT:  {{.*}}  %17 = MovInst %12 : number
//CHECK-NEXT:  {{.*}}  %18 = CompareBranchInst '>', %17 : number, %1 : number, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %19 = PhiInst %6 : number, %BB0, %16 : string|number|bigint, %BB1
//CHECK-NEXT:  {{.*}}  %20 = MovInst %19 : string|number|bigint
//CHECK-NEXT:  {{.*}}  %21 = ReturnInst %20 : string|number|bigint
//CHECK-NEXT:function_end
