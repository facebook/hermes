/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("in");
//CHECK-LABEL: in

try {
    1 in 2;
} catch (e) {
    print("caught:", e);
}
//CHECK-NEXT: caught: TypeError: right operand of 'in' is not an object

print(1 in {});
//CHECK-NEXT: false
print(1 in [10,20]);
//CHECK-NEXT: true
print(1 in [10]);
//CHECK-NEXT: false
print(1 in {});
//CHECK-NEXT: false
print(1 in {1:10});
//CHECK-NEXT: true
print(1 in {2:20});
//CHECK-NEXT: false
print("a" in {});
//CHECK-NEXT: false
print("a" in {a:10});
//CHECK-NEXT: true
