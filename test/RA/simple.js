/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheck %s --match-full-lines

//CHECK-LABEL:function main(x, y, z) : string|number|bigint
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %1 = HBCLoadParamInst 2 : number
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  {{.*}}  %3 = HBCLoadConstInst 0 : number
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadConstInst 10 : number
//CHECK-NEXT:  {{.*}}  %5 = MovInst %2 : number
//CHECK-NEXT:  {{.*}}  %6 = MovInst %3 : number
//CHECK-NEXT:  {{.*}}  %7 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %8 = PhiInst %5 : number, %BB0, %15 : string|number|bigint, %BB1
//CHECK-NEXT:  {{.*}}  %9 = PhiInst %6 : number, %BB0, %16 : number|bigint, %BB1
//CHECK-NEXT:  {{.*}}  %10 = BinaryOperatorInst '+', %0, %9 : number|bigint
//CHECK-NEXT:  {{.*}}  %11 = BinaryOperatorInst '+', %1, %9 : number|bigint
//CHECK-NEXT:  {{.*}}  %12 = BinaryOperatorInst '*', %10 : string|number|bigint, %11 : string|number|bigint
//CHECK-NEXT:  {{.*}}  %13 = BinaryOperatorInst '+', %8 : string|number|bigint, %12 : number|bigint
//CHECK-NEXT:  {{.*}}  %14 = UnaryOperatorInst '++', %9 : number|bigint
//CHECK-NEXT:  {{.*}}  %15 = MovInst %13 : string|number|bigint
//CHECK-NEXT:  {{.*}}  %16 = MovInst %14 : number|bigint
//CHECK-NEXT:  {{.*}}  %17 = CompareBranchInst '<', %16 : number|bigint, %4 : number, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %18 = ReturnInst %13 : string|number|bigint
//CHECK-NEXT:function_end

function main(x, y, z) {

  var sum = 3;
  for (var i = 0; i < 10; i++) {
    sum += (x + i) * (y + i);
  }

  return sum;
}
