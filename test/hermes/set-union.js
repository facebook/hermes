/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("set-union");
//CHECK-LABEL: set-union

var o1 = {foo: "bar"};
var o2 = {foo: "baz"};
var s1 = new Set([0, 1, 2, 3, o1, o2]);
var s2 = new Set([5, 4, 3, 2, o1]);

var union = s1.union(s2);
union.forEach(value => {
    print(value, value === o1, value === o2);
});
// CHECK-NEXT: 0 false false
// CHECK-NEXT: 1 false false
// CHECK-NEXT: 2 false false
// CHECK-NEXT: 3 false false
// CHECK-NEXT: [object Object] true false
// CHECK-NEXT: [object Object] false true
// CHECK-NEXT: 5 false false
// CHECK-NEXT: 4 false false

s1 = new Set([0, 1, 2])
s2.keys = function () {
    print("called keys");
    return [4, 3, 2].values();
}
union = s1.union(s2);
// CHECK-NEXT: called keys
union.forEach(value => {
    print(value);
});
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 4
// CHECK-NEXT: 3

s1 = new Set([1, 3]);
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
}

union = s1.union(setLikeObj);
// CHECK-NEXT: called keys
union.forEach(value => {
    print(value);
});
// CHECK-NEXT: 1
// CHECK-NEXT: 3
// CHECK-NEXT: 0
// CHECK-NEXT: 2

setLikeObj.keys = function() {
    print("called keys");
    s1.add(4);
    return [0, 2].values();
};

union = s1.union(setLikeObj);
// CHECK-NEXT: called keys
print(s1.size);
// CHECK-NEXT: 3

// The upfront copy of s1, used to derive the returned set, is taken after setLikeObj.keys
// is called, so the returned set will also contain the 4 added to s1 by setLikeObj.keys
union.forEach(value => {
    print(value);
});
// CHECK-NEXT: 1
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 0
// CHECK-NEXT: 2
