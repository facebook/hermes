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

/// Test Array.prototype.with with typed arrays.

'use strict';

(function () {

const nums: number[] = [1, 2, 3, 4, 5];

// Replace at index 2.
var w: number[] = nums.with(2, 99);
print(w.length, w[0], w[1], w[2], w[3], w[4]);
// CHECK: 5 1 2 99 4 5

// Original unchanged.
print(nums[2]);
// CHECK-NEXT: 3

// Replace at index 0.
w = nums.with(0, 10);
print(w[0], w[1]);
// CHECK-NEXT: 10 2

// Negative index.
w = nums.with(-1, 50);
print(w[4]);
// CHECK-NEXT: 50

// Replace last element.
w = nums.with(-2, 40);
print(w[3], w[4]);
// CHECK-NEXT: 40 5

// Out-of-bounds throws RangeError.
try { nums.with(5, 0); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: RangeError
try { nums.with(-6, 0); } catch (e) { print(e.constructor.name); }
// CHECK-NEXT: RangeError

})();
