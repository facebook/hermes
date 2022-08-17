/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

// Simple case
var x = {};
x.a = '1';
x.b = '2';
for (var y in x) {
  print(y, x[y]);
}
//CHECK: a 1
//CHECK: b 2


// Undefined object
x = undefined;
for (var y in x) {
  print(y, x[y]);
}

// Mixing array index and string properties
x = [];
x[0] = 1;
x['a'] = 2;
x[1] = 3;
for (var y in x) {
  print(y, x[y]);
}
//CHECK: 0 1
//CHECK: 1 3
//CHECK: a 2

// Prototypes
Array.prototype[0] = 'a';
Array.prototype[1] = 'b';
Array.prototype[2] = 'c';
x = new Array();
x['a'] = 'd';
x['1'] = 'e';
for (var y in x) {
  print(y, x[y]);
}
//CHECK: 1 e
//CHECK: a d
//CHECK: 0 a
//CHECK: 2 c

// NOTE: Use different property names for different tests to avoid unintentional
// sharing of hidden classes.

// Adding new properties
x = {'p0': 0};
for (var y in x) {
  print(y, x[y]);
}
//CHECK: p0 0
x.p1 =  1;
for (var y in x) {
  print(y, x[y]);
}
//CHECK: p0 0
//CHECK: p1 1

// Push back onto prototype chain
x = {'q0': 0};
for (var y in x) {
  print(y, x[y]);
}
//CHECK: q0 0
x.__proto__ = {'q1': 1};
for (var y in x) {
  print(y, x[y]);
}
//CHECK: q0 0
//CHECK: q1 1

// Pop back from prototype chain
x = {'r0': 0};
x.__proto__ = {'r1': 1};
for (var y in x) {
  print(y, x[y]);
}
//CHECK: r0 0
//CHECK: r1 1
x.__proto__ = null;
for (var y in x) {
  print(y, x[y]);
}
//CHECK: r0 0

// Mostly props in proto
x = {'s0': 0};
x.__proto__ = {'s1': 1, 's2': 2};
x.__proto__.__proto__ = {'s3': 3, 's4': 4, 's5': 5};
for (var y in x) {
  print(y, x[y]);
}
//CHECK: s0 0
//CHECK: s1 1
//CHECK: s2 2
//CHECK: s3 3
//CHECK: s4 4
//CHECK: s5 5

// Don't probe parent chain of non-cacheable objects (proxy will assert)
// 1. This creates a for-in cache entry for a particular hidden class
for (var k in Number(0)) {}
// 2. This Proxy uses the same hidden cache object internally.
for (var k in new Proxy({},{})) {}
