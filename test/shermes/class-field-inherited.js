/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

print('field inheritance');
// CHECK-LABEL: field inheritance

class C {
  x: number;
  constructor() {
    this.x = 1;
  }
}

class D extends C {
  constructor() {
    super();
  }
}

print(new C().x);
// CHECK-NEXT: 1
print(new D().x);
// CHECK-NEXT: 1

// Test implicit constructor.
class C2 {
  x: ?number;
}

class D2 extends C2 {
  constructor() {
    super();
  }
}

print(new C2().x);
// CHECK-NEXT: undefined
print(new D2().x);
// CHECK-NEXT: undefined
