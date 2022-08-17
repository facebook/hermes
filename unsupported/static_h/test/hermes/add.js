/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

"use strict";

print('addition');
// CHECK: addition

function add(x, y) {
  return x + y;
}

print(this['add'](1, 1));
// CHECK-NEXT: 2

print(this['add'](1.24, 7.11));
// CHECK-NEXT: 8.35

print(this['add']('1.24', 7.11));
// CHECK-NEXT: 1.247.11

print(this['add']('abc', 1));
// CHECK-NEXT: abc1

print(this['add'](13, 'abc'));
// CHECK-NEXT: 13abc

print(this['add']('abc', 'xyz'));
// CHECK-NEXT: abcxyz

print(this['add'](undefined, 1));
// CHECK-NEXT: NaN

print(this['add'](undefined, 'xyz'));
// CHECK-NEXT: undefinedxyz
