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

/// Test that typed rest parameters annotated as Array<T> produce a real
/// FastArray at runtime, so that subsequent FastArray ops (length, indexed
/// load, push, pop) work without a type-cast crash.

'use strict';

(function () {

function sum(...nums: Array<number>): number {
  var t: number = 0;
  for (var i: number = 0; i < nums.length; ++i) t += nums[i];
  return t;
}

print(sum());
// CHECK: 0
print(sum(1));
// CHECK-NEXT: 1
print(sum(1, 2, 3, 4));
// CHECK-NEXT: 10

// Mix a fixed param with a rest param.
function head(first: number, ...rest: Array<number>): number {
  return first + rest.length;
}
print(head(10));
// CHECK-NEXT: 10
print(head(10, 1, 2, 3));
// CHECK-NEXT: 13

// Rest param can be mutated like any other FastArray.
function pushBang(...xs: Array<string>): number {
  xs.push("!");
  return xs.length;
}
print(pushBang());
// CHECK-NEXT: 1
print(pushBang("a", "b"));
// CHECK-NEXT: 3

})();
