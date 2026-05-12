/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// Regression: JSON.parse must not cache a dictionary-mode HiddenClass in
// its per-depth shape cache. Dictionary classes are owned by exactly one
// object and mutated in place; sharing one between two sibling parsed
// objects with identical key sets lets a mutation on one corrupt the
// other (out-of-bounds property storage access).

// kDictionaryThreshold + 1: the slow-path object transitions to a dictionary.
var N = 65;

function mk(n) {
  var s = '{';
  for (var i = 0; i < n; i++) s += (i ? ',' : '') + '"k' + i + '":0';
  return s + '}';
}
var arr = JSON.parse('[' + mk(N) + ',' + mk(N) + ']');

// Deleting from arr[0] must not strip arr[1]'s property map.
delete arr[0].k1;
print(Object.keys(arr[1]).length);
// CHECK: 65
print(arr[1].k1);
// CHECK-NEXT: 0

// Adding to arr[0] must not extend arr[1]'s class (would be OOB on arr[1]).
arr = JSON.parse('[' + mk(N) + ',' + mk(N) + ']');
for (var i = 0; i < 8; i++) arr[0]['NEW' + i] = 'BAD';
print(arr[1].NEW0, arr[1].NEW7);
// CHECK-NEXT: undefined undefined
print(Object.keys(arr[1]).length);
// CHECK-NEXT: 65
