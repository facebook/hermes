/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

print("sort-sparse");
//CHECK: sort-sparse

// Check that a very sparse array can be sorted quickly.
var len = 10000000;
var a = {}
a.prop = "prop";
a[0] = 100;
a[5] = undefined;
a[len - 1] = 0;
a.length = len

pr(a);
//CHECK-NEXT: 0=100 5=undefined 9999999=0 prop=prop length=10000000
Array.prototype.sort.call(a);
pr(a);
//CHECK-NEXT: 0=0 1=100 2=undefined prop=prop length=10000000

// Make sure we can sort ordinary arrays too
pr([9,undefined,8,5,].sort());
//CHECK-NEXT: 0=5 1=8 2=9 3=undefined

// Sorting an empty object shouldn't crash or call the comparison function.
try {
  Array.prototype.sort.call({}, function() {
    throw new Error("shouldn't be called");
  });
  print("Didn't throw");
} catch (e) {
  print(e);
}
//CHECK-NEXT: Didn't throw

// A function to print an array/object.
function pr(x) {
    var s = "";
    for(var i in x) {
        s += " ";
        s += i;
        s += "=";
        s += x[i];
    }
    print(s);
}

