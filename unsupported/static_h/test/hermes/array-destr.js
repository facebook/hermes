/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

function *iter() {
  for (var i = 0; i < 5; ++i) {
    print(i);
    yield i;
  }
}

print('START');
// CHECK-LABEL: START

var [x,y] = iter();
// CHECK-NEXT: 0
// CHECK-NEXT: 1
print(x,y);
// CHECK-NEXT: 0 1

var [a,b,...[]] = iter();
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
print(a,b);
// CHECK-NEXT: 0 1

var [c,d,...[e,f]] = iter();
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
print(c,d,e,f);
// CHECK-NEXT: 0 1 2 3
