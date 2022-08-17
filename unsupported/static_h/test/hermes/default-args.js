/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

print('START');
// CHECK-LABEL: START

function foo(a, b=1) {
  print(a, b);
}

print(foo.length)
// CHECK-NEXT: 1
foo(1, 2);
// CHECK-NEXT: 1 2
foo(1);
// CHECK-NEXT: 1 1

function bar(a, b=1, c) {
  print(a, b);
}

print(bar.length)
// CHECK-NEXT: 1

function baz(a, b, c) {}

print(baz.length)
// CHECK-NEXT: 3
