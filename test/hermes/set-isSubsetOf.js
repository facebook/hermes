/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("set-isSubsetOf");
// CHECK-LABEL: set-isSubsetOf

var o1 = {foo: "bar"};
var o2 = {foo: "baz"};
var s1 = new Set([0, 1, 2, 3, 4, o1, o2]);
var s2 = new Set([6, 5, 4, 3, 2, o1]);

var subset = s1.isSubsetOf(s2);
print(subset);
// CHECK-NEXT: false

s1 = new Set([2, o2]);
subset = s1.isSubsetOf(s2);
print(subset);
// CHECK-NEXT: false

s1 = new Set([2, o1]);
subset = s1.isSubsetOf(s2);
print(subset);
// CHECK-NEXT: true

s1 = new Set([0, 1, 2, 3]);
// set-like object that encapulates the value [0, 1, 2] with size 3
var setLikeObj = {
  size: 3,
  has(val) {
    print("called has with val: ", val);
    return val === 0 || val === 1 || val === 2;
  },
  keys() {
    print("called keys");
    return [0, 1, 2].values();
  }
};

// s1.size > setLikeObj.size, so return false immediately without iterating
subset = s1.isSubsetOf(setLikeObj);
print(subset);
// CHECK-NEXT: false

s1 = new Set([0, 1]);
subset = s1.isSubsetOf(setLikeObj);
// CHECK-NEXT: called has with val: 0
// CHECK-NEXT: called has with val: 1
print(subset);
// CHECK-NEXT: true

s1 = new Set([0, 3, 1]);
subset = s1.isSubsetOf(setLikeObj);
// CHECK-NEXT: called has with val: 0
// CHECK-NEXT: called has with val: 3
print(subset);
// CHECK-NEXT: false

s1 = new Set([0, 1]);
setLikeObj.has = function(val) {
    print("called has with val: ", val);
    s1.add(3);
    return val === 0 || val === 1 || val === 2;
}

// 3 was added to s1 by the has method
subset = s1.isSubsetOf(setLikeObj);
// CHECK-NEXT: called has with val: 0
// CHECK-NEXT: called has with val: 1
// CHECK-NEXT: called has with val: 3
print(s1.size);
// CHECK-NEXT: 3
print(subset);
// CHECK-NEXT: false

setLikeObj.has = function(val) {
    print("called has with val: ", val);
    s1.delete(3);
    return val === 0 || val === 1 || val === 2;
}

// 3 was deleted from s1 by the has method
subset = s1.isSubsetOf(setLikeObj);
// CHECK-NEXT: called has with val: 0
// CHECK-NEXT: called has with val: 1
print(s1.size);
// CHECK-NEXT: 2
print(subset);
// CHECK-NEXT: true
