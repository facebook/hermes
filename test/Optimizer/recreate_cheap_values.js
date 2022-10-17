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
// CHECK: function poszero#0#1(f)#2
// CHECK:       %[[_2:[0-9]+]] = HBCLoadConstInst 0 : number
// CHECK-NEXT:  %{{[0-9]+}}    = ImplicitMovInst %[[_1:[0-9]+]] : undefined
// CHECK-NEXT:  %{{[0-9]+}}    = ImplicitMovInst %[[_2]] : number
// CHECK-NEXT:  %{{[0-9]+}}    = ImplicitMovInst %[[_2]] : number
// CHECK-NEXT:  %[[_6:[0-9]+]] = HBCCallNInst %{{[0-9]+}}, %[[_1]] : undefined, %[[_2]] : number, %[[_2]] : number
// CHECK-NEXT:  %{{[0-9]+}}    = ReturnInst %[[_6]]

// Negative zero is NOT 'cheap'.
function negzero(f) {
  return f(-0.0, -0.0);
}
// CHECK:function negzero#0#1(f)#2
// CHECK:       %[[_2:[0-9]+]] = HBCLoadConstInst -0 : number
// CHECK-NEXT:  %{{[0-9]+}}    = ImplicitMovInst %{{[0-9]+}} : undefined
// CHECK-NEXT:  %{{[0-9]+}}    = ImplicitMovInst %[[_2]] : number
// CHECK-NEXT:  %{{[0-9]+}}    = ImplicitMovInst %[[_2]] : number
