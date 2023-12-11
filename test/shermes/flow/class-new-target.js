/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O %s | %FileCheck --match-full-lines %s

'use strict';

class A {
  constructor() {
    print('A:', new.target === A);
    print('A:', new.target === B);
  }
}

class B extends A {
  constructor() {
    super();
    print('B:', new.target === A);
    print('B:', new.target === B);
  }
}

print('new target');
// CHECK-LABEL: new target
new A();
// CHECK-NEXT: A: true
// CHECK-NEXT: A: false
new B();
// CHECK-NEXT: A: false
// CHECK-NEXT: A: true
// CHECK-NEXT: B: false
// CHECK-NEXT: B: true
