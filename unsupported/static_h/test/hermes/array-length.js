/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -non-strict -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

if (typeof print === "undefined")
    var print = console.log;

function mustThrow(f) {
    try {
        f();
    } catch (e) {
        print("caught", e.name, e.message);
        return;
    }
    print("DID NOT THROW");
}

print("array-length");
//CHECK-LABEL: array-length

var a = Array(5);
print(a.length);
//CHECK-NEXT: 5

a.length = null;
print(a.length);
//CHECK-NEXT: 0

a.length = 2;
print(a.length);
//CHECK-NEXT: 2

mustThrow(function(){ a.length = "foo"; });
//CHECK-NEXT: caught RangeError Invalid array length
print(a.length);
//CHECK-NEXT: 2

mustThrow(function(){ a.length = 3.14; });
//CHECK-NEXT: caught RangeError Invalid array length
print(a.length);
//CHECK-NEXT: 2

mustThrow(function(){ Object.defineProperty(a, "length", {value:3.14}); });
//CHECK-NEXT: caught RangeError Invalid array length
print(a.length);
//CHECK-NEXT: 2

// Make ".length" read-only and try to extend the array
Object.defineProperty(a, "length", {writable:false});

a[0]=0;
a[1]=1;
a[2]=2;
print(a.length, a[0], a[1], a[2]);
//CHECK-NEXT: 2 0 1 undefined

mustThrow(function(){ "use strict"; a[3] = 3;});
//CHECK-NEXT: caught TypeError {{.*}}
print(a.length, a[0], a[1], a[3]);
//CHECK-NEXT: 2 0 1 undefined

// Make sure .length is not an accessor.
print("getter", Object.getOwnPropertyDescriptor(a, "length").get);
//CHECK-NEXT: getter undefined

var child = {}
child.__proto__ = [1,2];
child.length = 10;
print(child.length, child.__proto__.length);
//CHECK-NEXT: 10 2

var arr = [];
arr[5] = 0;
arr.length = 0;
print(arr.length, arr[5]);
//CHECK-NEXT: 0 undefined

// Check that shrinking the length on a big non-standard array works
arr = [];
for (var i = 0; i < 50; i++) {
  Object.defineProperty(arr, i, {enumerable: false, configurable: true, value: i});
}
print(arr.length);
//CHECK-NEXT: 50
arr.length = 0;
print(arr.length);
//CHECK-NEXT: 0

// Ensure that we can make the length read-only while changing it in the same
// operation.
var a = [];
Object.defineProperty(a, "length", {value:2, writable: false});
print(a.length);
//CHECK-NEXT: 2

// Make sure that setting length as a computed property works.
var a = [1, 2, 3, 4, 5]
print(a);
//CHECK-NEXT: 1,2,3,4,5
var t = "length";
a[t] = 3;
print(a);
//CHECK-NEXT: 1,2,3
a[t] = 5;
print(a);
//CHECK-NEXT: 1,2,3,,
