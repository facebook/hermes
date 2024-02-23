/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

print("check array.at");
// CHECK: check array.at

let arr = [10, 20, 30];
arr.__proto__[1] = 200;

print(JSON.stringify(arr));
// CHECK-NEXT: [10,20,30]
print(arr.at(0), arr.at(1), arr.at(2));
// CHECK-NEXT: 10 20 30
delete arr[1];

// Now that the array contains an empty element, it should hit 
// the property defined on the prototype chain.
print(JSON.stringify(arr));
// CHECK-NEXT: [10,200,30]
print(arr.at(0), arr.at(1), arr.at(2));
// CHECK-NEXT: 10 200 30
