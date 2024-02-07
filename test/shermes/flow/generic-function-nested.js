/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

function outer<T>(x: T): T {
  var innerVar: number = 3;
  function inner<U>(y: U): U {
    {
      let z: T = x;
      print(z);
    }
    return y;
  }
  return inner<T>(x);
}

print('generic function');
// CHECK-LABEL: generic function

print(outer<number>(10));
// CHECK: 10
// CHECK-NEXT: 10
print(outer<string>('abc'));
// CHECK-NEXT: abc
// CHECK-NEXT: abc
