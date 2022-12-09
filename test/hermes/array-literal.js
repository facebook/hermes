/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// Test that literal arrays work.
"use strict";

print('Array Literal');
// CHECK-LABEL: Array Literal
var arr = [3, 5, null, true, false];
print(arr.__proto__.constructor === Array);
// CHECK-NEXT: true
print(arr.toString());
// CHECK-NEXT: 3,5,,true,false
print(arr[0]);
// CHECK-NEXT: 3
print(arr[4]);
// CHECK-NEXT: false
