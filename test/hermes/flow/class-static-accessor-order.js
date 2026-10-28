/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Werror -typed -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

// A static accessor whose setter is declared before its getter: each half
// must be called through its own closure.
class A {
  static _v: number = 5;
  @Hermes.final static set x(v: number) {
    print('set', v);
    A._v = v;
  }
  @Hermes.final static get x(): number {
    print('get');
    return A._v;
  }
}

print(A.x);
// CHECK: get
// CHECK-NEXT: 5
A.x = 7;
// CHECK-NEXT: set 7
print(A.x);
// CHECK-NEXT: get
// CHECK-NEXT: 7
