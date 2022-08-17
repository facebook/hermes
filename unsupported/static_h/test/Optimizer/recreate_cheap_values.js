/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-postra -O %s | %FileCheck %s --match-full-lines

// Positive zero is 'cheap'.
function poszero(f) {
  return f(0.0, 0.0);
}
// CHECK: function poszero(f)
// CHECK:   %2 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  %3 = ImplicitMovInst %1 : undefined
// CHECK-NEXT:  %4 = ImplicitMovInst %2 : number
// CHECK-NEXT:  %5 = ImplicitMovInst %2 : number
// CHECK-NEXT:  %6 = HBCCallNInst %0, %1 : undefined, %2 : number, %2 : number
// CHECK-NEXT:  %7 = ReturnInst %6

// Negative zero is NOT 'cheap'.
function negzero(f) {
  return f(-0.0, -0.0);
}
// CHECK:function negzero(f)
// CHECK:  %2 = HBCLoadConstInst -0 : number
// CHECK-NEXT:  %3 = ImplicitMovInst %1 : undefined
// CHECK-NEXT:  %4 = ImplicitMovInst %2 : number
// CHECK-NEXT:  %5 = ImplicitMovInst %2 : number
