/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

class C {
  x: number = 10;
  method(): number {
    return this.x;
  }
  inherited(): number {
    return 1;
  }
}

class D extends C {
  method(): number {
    return this.x + 1;
  }
  own(): number {
    return 2;
  }
}

print('class prototype');
// CHECK-LABEL: class prototype

const c = new C();
const d = new D();

print(C.prototype === Object.getPrototypeOf(c));
// CHECK-NEXT: true
print(D.prototype === Object.getPrototypeOf(d));
// CHECK-NEXT: true
print(typeof C.prototype.method);
// CHECK-NEXT: function

print(C.prototype.method.call(c));
// CHECK-NEXT: 10
print(D.prototype.method.call(d));
// CHECK-NEXT: 11
print(D.prototype.inherited.call(d));
// CHECK-NEXT: 1
print(D.prototype.own.call(d));
// CHECK-NEXT: 2

// Reading through C.prototype selects C's method statically; it is not
// dispatched on the receiver.
print(C.prototype.method.call(d));
// CHECK-NEXT: 10
