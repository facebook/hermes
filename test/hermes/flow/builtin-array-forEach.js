/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

/// Test Array.prototype.forEach with typed arrays.

'use strict';

(function () {

const nums: number[] = [10, 20, 30];

// Basic forEach.
var sum: number = 0;
nums.forEach((n: number): void => { sum += n; });
print(sum);
// CHECK: 60

// forEach with index parameter.
var indexSum: number = 0;
nums.forEach((n: number, i: number): void => { indexSum += i; });
print(indexSum);
// CHECK-NEXT: 3

// forEach on empty array does nothing.
var called: boolean = false;
const empty: number[] = [];
empty.forEach((n: number): void => { called = true; });
print(called);
// CHECK-NEXT: false

// forEach with thisArg.
var ctx: number[] = [100];
var result: number = 0;
nums.forEach(function(this: number[], n: number): void {
  result += this[0] + n;
}, ctx);
print(result);
// CHECK-NEXT: 360

})();
