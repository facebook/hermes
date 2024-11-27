/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

let obj = { a: 5 };
with (obj) {
    print(a);
}
// CHECK: 5

obj = { b: 10, nested: { c: 15 } };
with (obj) {
    print(b);
    with (nested) {
        print(c);
    }
}
// CHECK-NEXT: 10
// CHECK-NEXT: 15

obj = { d: 20 };
with (obj) {
    delete d;
}
print('d' in obj);
// CHECK-NEXT: false

obj = { e: 25 };
with (obj) {
    e = 30;
}
print(obj.e);
// CHECK-NEXT: 30
