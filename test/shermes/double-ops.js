/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck %s --match-full-lines

print('doubles');
// CHECK-LABEL: doubles

function math(a, b) {
  a = +a;
  b = +b;

  print(a + b);
  print(a - b);
  print(a * b);
  print(a / b);
  print(a % b);
  print(a | 0);
}

math(1, 2);
// CHECK-NEXT: 3
// CHECK-NEXT: -1
// CHECK-NEXT: 2
// CHECK-NEXT: 0.5
// CHECK-NEXT: 1
// CHECK-NEXT: 1

function compare(a, b) {
  a = +a;
  b = +b;

  print(a < b);
  print(a > b);
  print(a <= b);
  print(a >= b);
  print(a === b);
  print(a !== b);
  print(a == b);
  print(a != b);
}

compare(1, 2);
// CHECK-NEXT: true
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: false
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: false
// CHECK-NEXT: true

function branch(a, b) {
  a = +a;
  b = +b;

  if(a < b)
    print('less');
  if(a > b)
    print('greater');
  if(a <= b)
    print('less_equal');
  if(a >= b)
    print('greater_equal');
  if(a === b)
    print('strict_equal');
  if(a !== b)
    print('strict_not_equal');
  if(a == b)
    print('equal');
  if(a != b)
    print('not_equal');
}

branch(1, 2);
// CHECK-NEXT: less
// CHECK-NEXT: less_equal
// CHECK-NEXT: strict_not_equal
// CHECK-NEXT: not_equal

print('done');
// CHECK-NEXT: done
