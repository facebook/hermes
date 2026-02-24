/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("set-symmetricDifference");
// CHECK-LABEL: set-symmetricDifference

var o1 = {foo: "bar"};
var o2 = {foo: "baz"};
var o3 = {foo: "qux"}
var s1 = new Set([0, 1, o1, o2]);
var s2 = new Set([1, 3, o2, o3]);

var symDiff = s1.symmetricDifference(s2);
symDiff.forEach(value => {
    print(value, value === o1, value === o3);
});
// CHECK-NEXT: 0 false false
// CHECK-NEXT: [object Object] true false
// CHECK-NEXT: 3 false false
// CHECK-NEXT: [object Object] false true

s1 = new Set([1, 3, o2, o3]);
symDiff = s1.symmetricDifference(s2);
print(symDiff.size);
// CHECK-NEXT: 0

s1 = new Set([0, 1]);
s2 = new Set([2, 3]);
s2.keys = function () {
    print("called keys");
    return [0, 1].values();
}
symDiff = s1.symmetricDifference(s2);
// CHECK-NEXT: called keys
// s2.keys will iterate over [0, 1] instead of the set elements [2, 3]
// so the expected result should be empty
print(symDiff.size);
// CHECK-NEXT: 0

s1 = new Set([1, 2, 3, 5]);
// set-like object that encapulates the value [0, 2] with size 2
var setLikeObj = {
    size: 2,
    has(val) {
        print("called has with val: ", val);
        return val === 0 || val === 2;
    },
    keys() {
        print("called keys");
        return [0, 2].values();
    }
};

symDiff = s1.symmetricDifference(setLikeObj);
// CHECK-NEXT: called keys
symDiff.forEach(value => {
    print(value);
});
// CHECK-NEXT: 1
// CHECK-NEXT: 3
// CHECK-NEXT: 5
// CHECK-NEXT: 0

setLikeObj.keys = function() {
    print("called keys");
    s1.add(4);
    return [0, 2].values();
};

symDiff = s1.symmetricDifference(setLikeObj);
// CHECK-NEXT: called keys
print(s1.size);
// CHECK-NEXT: 5

// The upfront copy of s1, used to derive the returned set, is taken after setLikeObj.keys
// is called, so the returned set will also contain the 4 added to s1 by setLikeObj.keys
symDiff.forEach(value => {
    print(value);
});
// CHECK-NEXT: 1
// CHECK-NEXT: 3
// CHECK-NEXT: 5
// CHECK-NEXT: 4
// CHECK-NEXT: 0
