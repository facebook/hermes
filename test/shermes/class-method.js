/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

print('classes');
// CHECK-LABEL: classes

class NoFieldsNoCons {
  method(): number {
    return 1;
  }
}

print(new NoFieldsNoCons() instanceof NoFieldsNoCons);
// CHECK-NEXT: true
print(new NoFieldsNoCons().method());
// CHECK-NEXT: 1

class NoFields {
  constructor() {
    print('cons');
  }
  method(): number {
    return 2;
  }
}

print(new NoFields().method());
// CHECK-NEXT: cons
// CHECK-NEXT: 2 

class WithFields {
  x: number;
  constructor() {
    print('cons');
    this.x = 3;
  }
  method(): number {
    return this.x;
  }
}

print(new WithFields().method());
// CHECK-NEXT: cons
// CHECK-NEXT: 3
