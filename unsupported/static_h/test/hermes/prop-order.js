/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Verify that index-like properties don't create aliases.
var a = []
var tmp = {}
tmp.writable = false;
tmp.enumerable = true;
Object.defineProperty(a, "0", tmp);
for(var i in a)
    print(i);
print("end1");
//CHECK: 0
//CHECK-NEXT: end1

// Verify that property names are sorted according to ES2015
var x = {}
x.c = 1;
x[2] = 1;
x.b = 1;
x[1] = 1;

for(var i in x)
    print(i);
print("end2");
//CHECK-NEXT: 1
//CHECK-NEXT: 2
//CHECK-NEXT: c
//CHECK-NEXT: b
//CHECK-NEXT: end2

var a = [,1,1,,1]
a.c = 1;
a.b = 1;
var tmp = {}
tmp.writable = false;
tmp.enumerable = true;
Object.defineProperty(a, "0", tmp);
Object.defineProperty(a, "3", tmp);
for(var i in a)
    print(i);
print("end3");
//CHECK-NEXT: 0
//CHECK-NEXT: 1
//CHECK-NEXT: 2
//CHECK-NEXT: 3
//CHECK-NEXT: 4
//CHECK-NEXT: c
//CHECK-NEXT: b
//CHECK-NEXT: end3
