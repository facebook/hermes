/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

print('start');
// CHECK-LABEL: start

function head<T>(x: T[]): T {
  return x[0];
}
print(head([1, 2, 3]));
// CHECK-NEXT: 1
print(head(['a', 'b', 'c']));
// CHECK-NEXT: a
let b: boolean[] = [true, true, false];
print(head(b));
// CHECK-NEXT: true

function first<T, U>(x: [T, U]): T {
  return x[0];
}
print(first([1, 2]));
// CHECK-NEXT: 1
let c: [boolean, boolean] = [false, true];
print(first(c));
// CHECK-NEXT: false

function combine<T, U>(x: T, y: U): [T, U] {
  return [x, y];
}
let d: [number, boolean];
d = combine(1, true);
print(d[0], d[1]);
// CHECK-NEXT: 1 true
