/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("set-isDisjointFrom");
// CHECK-LABEL: set-isDisjointFrom

o1 = {foo: "bar"};
o2 = {foo: "baz"};
var s1 = new Set([0, 1, 2, 3, o1, o2]);
var s2 = new Set([6, 5, 4, 3, 2, o1]);

var disjoint = s1.isDisjointFrom(s2);
print(disjoint);
// CHECK-NEXT: false

s1 = new Set([0, 1, o2]);
disjoint = s1.isDisjointFrom(s2);
print(disjoint);
// CHECK-NEXT: true

s2.has = function (val) {
    print("called has with val: ", val);
    return true;
}
disjoint = s1.isDisjointFrom(s2);
// CHECK-NEXT: called has with val: 0

// s2.has will always return true, so even if s1 and s2 are disjoint sets,
// expect the returned value to be false
print(disjoint)
// CHECK-NEXT: false

s2 = new Set([2, 3]);
s2.keys = function () {
    print("called keys");
    return [0, 1].values();
}
disjoint = s1.isDisjointFrom(s2);
// CHECK-NEXT: called keys
// s2.keys will iterate through [0, 1] instead of the its elements [2, 3], so
// the expected result is false, even if the two sets are disjoint
print(disjoint);
// CHECK-NEXT: false

s1 = new Set([1, 3]);
// set-like object that encapulates the value [2, 4, 6] with size 3
var setLikeObj = {
    size: 3,
    has(val) {
        print("called has with val: ", val);
        return val === 2 || val === 4 || val === 6;
    },
    keys() {
        print("called keys");
        return [2, 4, 6].values();
    }
}

disjoint = s1.isDisjointFrom(setLikeObj);
// CHECK-NEXT: called has with val: 1
// CHECK-NEXT: called has with val: 3
print(disjoint);
// CHECK-NEXT: true

s1 = new Set([1, 2, 3]);
disjoint = s1.isDisjointFrom(setLikeObj);
// CHECK-NEXT: called has with val: 1
// CHECK-NEXT: called has with val: 2
print(disjoint);
// CHECK-NEXT: false

s1 = new Set([1, 3, 5, 7]);
disjoint = s1.isDisjointFrom(setLikeObj);
// CHECK-NEXT: called keys
print(disjoint);
// CHECK-NEXT: true

s1 = new Set([1, 3, 5, 6]);
disjoint = s1.isDisjointFrom(setLikeObj);
// CHECK-NEXT: called keys
print(disjoint);
// CHECK-NEXT: false


s1 = new Set([1, 4, 5]);
setLikeObj.has = function(val) {
    print("called has with val: ", val);
    s1.delete(4);
    return val === 2 || val === 4 || val === 6;
};

// s1.size <= setLikeObj.size, so iterate through s1 directly and call setLikeObj.has
// This means that setLikeObj.has will delete 4 from s1 and isDisjointFrom will return true
disjoint = s1.isDisjointFrom(setLikeObj);
// CHECK-NEXT: called has with val: 1
// CHECK-NEXT: called has with val: 5
print(disjoint);
// CHECK-NEXT: true
s1.forEach(value => {
    print(value)
});
// CHECK-NEXT: 1
// CHECK-NEXT: 5

s1 = new Set([1, 3, 5, 7]);
setLikeObj.keys = function() {
    print("called keys");
    s1.add(4);
    return [2, 4, 6].values();
};

// s1.size > setLikeObj.size, so iterate through setLikeObj.keys
// setLikeObj.keys will add 4 to s1, so the sets are not disjoint
disjoint = s1.isDisjointFrom(setLikeObj);
// CHECK-NEXT: called keys
print(disjoint);
// CHECK-NEXT: false
