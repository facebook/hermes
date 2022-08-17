/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print(10);
//CHECK: 10
print(10.5);
//CHECK-NEXT: 10.5
print("hello");
//CHECK-NEXT: hello
print(void 0);
//CHECK-NEXT: undefined
print(null);
//CHECK-NEXT: null
print(true);
//CHECK-NEXT: true
print(false);
//CHECK-NEXT: false
print(0);
//CHECK-NEXT: 0
