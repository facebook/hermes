/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -typed %s | %FileCheck --match-full-lines %s
// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

/// Test the Number method trampolines in TypedLib.

'use strict';

(function () {

const n: number = 3.14159;
const neg: number = -2.5;
const i: number = 255;

print('toFixed');
// CHECK-LABEL: toFixed
print(n.toFixed(2), n.toFixed(0));
// CHECK-NEXT: 3.14 3
print(neg.toFixed(1), i.toFixed());
// CHECK-NEXT: -2.5 255
// Rounding away from zero at the halfway point.
print((1.005).toFixed(1), (0.5).toFixed(0));
// CHECK-NEXT: 1.0 1

print('toPrecision');
// CHECK-LABEL: toPrecision
print(n.toPrecision(3), i.toPrecision(2));
// CHECK-NEXT: 3.14 2.6e+2
// No argument behaves like toString().
print(n.toPrecision());
// CHECK-NEXT: 3.14159

print('toExponential');
// CHECK-LABEL: toExponential
print(i.toExponential(2), neg.toExponential(1));
// CHECK-NEXT: 2.55e+2 -2.5e+0

print('toString');
// CHECK-LABEL: toString
print(n.toString(), i.toString());
// CHECK-NEXT: 3.14159 255
print(i.toString(2), i.toString(16));
// CHECK-NEXT: 11111111 ff
print(neg.toString(), neg.toString(2));
// CHECK-NEXT: -2.5 -10.1

})();
