/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheck %s --match-full-lines

//CHECK-LABEL:function main(x, y, z) : string|number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %1 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  {{.*}}  %3 = HBCLoadConstInst 0 : number
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  {{.*}}  %5 = HBCLoadConstInst 10 : number
//CHECK-NEXT:  {{.*}}  %6 = MovInst %2 : number
//CHECK-NEXT:  {{.*}}  %7 = MovInst %3 : number
//CHECK-NEXT:  {{.*}}  %8 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %9 = PhiInst %6 : number, %BB0, %16 : string|number, %BB1
//CHECK-NEXT:  {{.*}}  %10 = PhiInst %7 : number, %BB0, %17 : number, %BB1
//CHECK-NEXT:  {{.*}}  %11 = BinaryOperatorInst '+', %0, %10 : number
//CHECK-NEXT:  {{.*}}  %12 = BinaryOperatorInst '+', %1, %10 : number
//CHECK-NEXT:  {{.*}}  %13 = BinaryOperatorInst '*', %11 : string|number, %12 : string|number
//CHECK-NEXT:  {{.*}}  %14 = BinaryOperatorInst '+', %9 : string|number, %13 : number
//CHECK-NEXT:  {{.*}}  %15 = BinaryOperatorInst '+', %10 : number, %4 : number
//CHECK-NEXT:  {{.*}}  %16 = MovInst %14 : string|number
//CHECK-NEXT:  {{.*}}  %17 = MovInst %15 : number
//CHECK-NEXT:  {{.*}}  %18 = CompareBranchInst '<', %17 : number, %5 : number, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %19 = ReturnInst %14 : string|number
//CHECK-NEXT:function_end

function main(x, y, z) {

  var sum = 3;
  for (var i = 0; i < 10; i++) {
    sum += (x + i) * (y + i);
  }

  return sum;
}
