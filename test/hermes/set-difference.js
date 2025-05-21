/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("set-difference");
// CHECK-LABEL: set-difference

o1 = {foo: "bar"};
o2 = {foo: "baz"};
var s1 = new Set([0, 1, 2, 3, o1, o2]);
var s2 = new Set([6, 5, 4, 3, 2, o1]);

var diff = s1.difference(s2)
diff.forEach(value => {
    print(value, value === o2);
});
// CHECK-NEXT: 0 false
// CHECK-NEXT: 1 false
// CHECK-NEXT: [object Object] true

s1 = new Set([6, 5, 4]);
diff = s1.difference(s2);
print(diff.size);
// CHECK-NEXT: 0

s2 = new Set([2, 4]);
diff = s1.difference(s2);
diff.forEach(value => {
    print(value);
});
// CHECK-NEXT: 6
// CHECK-NEXT: 5

s1 = new Set([1, 2]);
s2 = new Set([1, 2]);

diff = s1.difference(s2);
print(diff.size);
// CHECK-NEXT: 0

s2.has = function() {
    return false;
}
diff = s1.difference(s2);
// s2.has was overwritten to always return false, so the expected results
// should be everything in s1
diff.forEach(value => {
    print(value)
})
// CHECK-NEXT: 1
// CHECK-NEXT: 2

s1 = new Set([1, 2, 3]);
s2 = new Set([1, 2]);
s2.keys = function() {
    return [3, 4].values();
}
diff = s1.difference(s2);
// s2.keys has been overwritten to iterate through [3, 4], so the expected results
// should contain [1, 2]
diff.forEach(value => {
    print(value);
})
// CHECK-NEXT: 1
// CHECK-NEXT: 2

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
        return [0, 2].values()
    }
}

// Iterate through a copy of s1 and call setLikeObj.has because s1.size <= setLikeObj.size
diff = s1.difference(setLikeObj);
// CHECK-NEXT: called has with val: 1
// CHECK-NEXT: called has with val: 2
diff.forEach(value => {
    print(value)
});
// CHECK-NEXT: 1

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
diff = s1.difference(setLikeObj);
// CHECK-NEXT: called keys
diff.forEach(value => {
    print(value)
});
// CHECK-NEXT: 1
// CHECK-NEXT: 3

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
