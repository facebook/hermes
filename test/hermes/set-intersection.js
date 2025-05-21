/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("set-intersection");
// CHECK-LABEL: set-intersection

o1 = {foo: "bar"};
o2 = {foo: "baz"};
var s1 = new Set([0, 1, 2, 3, o1, o2]);
var s2 = new Set([6, 5, 4, 3, 2, o1]);

var intersect = s1.intersection(s2)
intersect.forEach(value => {
    print(value, value === o1);
});
// CHECK-NEXT: 2 false
// CHECK-NEXT: 3 false
// CHECK-NEXT: [object Object] true

s1 = new Set([0, 1]);
intersect = s1.intersection(s2)
print(intersect.size);
// CHECK-NEXT: 0

s1 = new Set([1, 2]);
// set-like object that encapulates the value [0, 2] with size 2
var setLikeObj = {
    size: 2,
    has(val) {
        print("called has with val: ", val);
        s1.add(3);
        return val === 0 || val === 2;
    },
    keys() {
        print("called keys");
        s1.add(4);
        return [0, 2].values();
    }
}

// Iterate through a copy of s1 and call setLikeObj.has because s1.size <= setLikeObj.size
intersect = s1.intersection(setLikeObj);
// CHECK-NEXT: called has with val: 1
// CHECK-NEXT: called has with val: 2
// CHECK-NEXT: called has with val: 3
intersect.forEach(value => {
    print(value)
});
// CHECK-NEXT: 2

// 3 is added in s1 by setLikeObj.has
print(s1.size);
// CHECK-NEXT: 3
s1.forEach(value => {
    print(value)
});
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3

// Iterate through the setLikeObject using keys because s1.size > setLikeObj.size
intersect = s1.intersection(setLikeObj);
// CHECK-NEXT: called keys
intersect.forEach(value => {
    print(value)
});
// CHECK-NEXT: 2

// 4 is added in s1 by setLikeObj.keys
print(s1.size);
// CHECK-NEXT: 4
s1.forEach(value => {
    print(value)
});
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

s1 = new Set([0, 1, 2]);
// set-like object that encapulates the value [0, 2, 4] with size 3
setLikeObj = {
    size: 3,
    has(val) {
        print("called has with val: ", val);
        s1.add(4);
        return val === 0 || val === 2 || val === 4;
    },
    keys() {
        print("called keys");
        s1.add(0);
        return [0, 2, 4].values();
    }
}

intersect = s1.intersection(setLikeObj);
// CHECK-NEXT: called has with val: 0
// CHECK-NEXT: called has with val: 1
// CHECK-NEXT: called has with val: 2
// CHECK-NEXT: called has with val: 4
intersect.forEach(value => {
    print(value);
});
// CHECK-NEXT: 0
// CHECK-NEXT: 2
// CHECK-NEXT: 4

// 3 is added to s1 by setLikeObj.has
print(s1.size);
// CHECK-NEXT: 4

setLikeObj.keys = function() {
    print("called keys");
    s1.delete(0);
    return [0, 2, 4].values();
}
intersect = s1.intersection(setLikeObj);
// CHECK-NEXT: called keys
intersect.forEach(value => {
    print(value);
});
// CHECK-NEXT: 2
// CHECK-NEXT: 4

// 4 is deleted from s1 by setLikeObj.keys
print(s1.size);
// CHECK-NEXT: 3
