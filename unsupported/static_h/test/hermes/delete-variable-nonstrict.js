/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Wno-direct-eval %s | %FileCheck --match-full-lines %s

a = 10;
print(a);
//CHECK: 10

print(delete a);
//CHECK-NEXT: true

print(delete a);
//CHECK-NEXT: true

print(this.a);
//CHECK-NEXT: undefined

try {
    print(a);
} catch (e) {
    print("caught", e.name, e.message);
}
//CHECK-NEXT: caught ReferenceError {{.*}}

var b = 20;
print(b);
//CHECK-NEXT: 20

print(delete(b));
//CHECK-NEXT: false

print(b);
//CHECK-NEXT: 20

print(eval("delete b;"))
//CHECK-NEXT: false
