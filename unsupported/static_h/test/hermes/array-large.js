/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("Array.prototype.join with a large array should throw RangeError")
// CHECK-LABEL: Array.prototype.join with a large array should throw RangeError

var v = [];
// This number is chosen carefully to be 1 more than the max elements that
// JSArray supports with MallocGC.
v.length = 4294966272 + 1;

try {
  // Trying to stringify this leads to Array.join() trying to allocate an
  // array that is too large.
  ++v;
  print("Succeeded");
} catch (e) {
  print("Caught", e.name, e.message);
}
//CHECK: Caught{{.*}}

print("Array.prototype.join with a large string should throw RangeError")
// CHECK-LABEL: Array.prototype.join with a large string should throw RangeError
var str = " ";
for(var i = 0; i < 20; ++i)
  str += str;

v = [];
v.length = 500;
try {
  // This produces a 500MB string, which should throw an exception.
  v.join(str);
  print("Succeeded");
} catch (e) {
  print("Caught", e.name, e.message);
}
//CHECK-NEXT: Caught{{.*}}

print("Array.prototype.toLocaleString with an array of large strings should throw RangeError")
// CHECK-LABEL: Array.prototype.toLocaleString with an array of large strings should throw RangeError
var a = 'a';
for (var i = 0; i < 28; ++i) {
  a = a + a;
}
print(a.length);
// CHECK-NEXT: 268435456
try {
  [a, a, a].toLocaleString();
  print("Succeeded");
} catch (e) {
  print("Caught", e.name, e.message);
}
//CHECK-NEXT: Caught RangeError{{.*}}
