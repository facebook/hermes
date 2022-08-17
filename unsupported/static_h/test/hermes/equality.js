/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var a = "10";
var b = 10;
var c = 20;
var d = 11;

print("start"); // Prevent constant propagation
//CHECK: start

print(a == b);
//CHECK-NEXT: true
print(a != c);
//CHECK-NEXT: true

--d;
print(a !== b);
//CHECK-NEXT: true
print(b === d);
//CHECK-NEXT: true
