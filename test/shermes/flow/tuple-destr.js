/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O0 -typed -exec %s | %FileCheck --match-full-lines %s

print('tuple');
// CHECK-LABEL: tuple

var tup: [number, number] = [1, 2];

class C {
  x: number;
  y: number;
  constructor() {
    this.x = 10;
    this.y = 10;
  }
}

var c: C = new C();

function getC(): C {
  tup[0]++;
  return c;
}


[getC().x, getC().y] = tup;
print(c.x, c.y);
// CHECK-NEXT: 2 2

tup = ([3, 4]: [number, number]);
let [a, b] = tup;
print(a, b);
// CHECK-NEXT: 3 4

tup = ([5, 6]: [number, number]);
[a, b] = tup;
print(a, b);
// CHECK-NEXT: 5 6
