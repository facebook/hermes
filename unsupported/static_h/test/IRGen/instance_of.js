/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -dump-ra %s -O

//CHECK-LABEL:function simple_test0(x, y)
//CHECK-NEXT:frame = [x, y]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:   {{.*}} %0 = HBCCreateEnvironmentInst
//CHECK-NEXT:   {{.*}} %1 = HBCLoadParamInst 1 : number
//CHECK-NEXT:   {{.*}} %2 = HBCLoadParamInst 2 : number
//CHECK-NEXT:   {{.*}} %3 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:   {{.*}} %4 = HBCStoreToEnvironmentInst %0, %1, [x]
//CHECK-NEXT:   {{.*}} %5 = HBCStoreToEnvironmentInst %0, %2, [y]
//CHECK-NEXT:   {{.*}} %6 = HBCLoadFromEnvironmentInst %0, [x]
//CHECK-NEXT:   {{.*}} %7 = HBCLoadFromEnvironmentInst %0, [y]
//CHECK-NEXT:   {{.*}} %8 = BinaryOperatorInst 'instanceof', %6, %7
//CHECK-NEXT:   {{.*}} %9 = ReturnInst %8
//CHECK-NEXT:%BB1:
//CHECK-NEXT:   $???   %10 = ReturnInst %3 : undefined
//CHECK-NEXT:function_end

function simple_test0(x, y) {
  return x instanceof y;
}
