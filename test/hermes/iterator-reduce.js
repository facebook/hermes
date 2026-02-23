/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

print('Iterator.prototype.reduce');
// CHECK-LABEL: Iterator.prototype.reduce

// Non-callable reducer should throw TypeError before consuming the iterator.
var iter = (function* () { yield 1; })();
try {
  iter.reduce({});
} catch (e) {
  print(e instanceof TypeError);
}
// CHECK-NEXT: true

// The iterator should not have been consumed by the callable check.
var result = iter.next();
print(result.value, result.done);
// CHECK-NEXT: 1 false

// Basic reduce with initial value.
var sum = [1, 2, 3].values().reduce(function(acc, v) { return acc + v; }, 0);
print(sum);
// CHECK-NEXT: 6

// Reduce without initial value uses first element as accumulator.
var sum = [1, 2, 3].values().reduce(function(acc, v) { return acc + v; });
print(sum);
// CHECK-NEXT: 6

// Reduce of empty iterator with no initial value should throw TypeError.
try {
  [].values().reduce(function(acc, v) { return acc + v; });
} catch (e) {
  print(e instanceof TypeError);
}
// CHECK-NEXT: true
