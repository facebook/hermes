/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

print("toSorted");
// CHECK-LABEL: toSorted

// Basic numeric sort with comparefn.
(function () {
  var arr = [3, 1, 4, 1, 5, 9, 2, 6];
  var sorted = arr.toSorted(function (a, b) { return a - b; });
  print(sorted.join(","));
  // CHECK-NEXT: 1,1,2,3,4,5,6,9
  // Original unchanged.
  print(arr.join(","));
  // CHECK-NEXT: 3,1,4,1,5,9,2,6
})();

// Default lexicographic sort (no comparefn).
(function () {
  var arr = [10, 9, 8, 1, 20, 3];
  var sorted = arr.toSorted();
  print(sorted.join(","));
  // CHECK-NEXT: 1,10,20,3,8,9
})();

// Empty array.
(function () {
  var arr = [];
  var sorted = arr.toSorted();
  print(sorted.length);
  // CHECK-NEXT: 0
})();

// Single element.
(function () {
  var arr = [42];
  var sorted = arr.toSorted();
  print(sorted.length, sorted[0]);
  // CHECK-NEXT: 1 42
})();

// Holes become undefined and sort to the end.
(function () {
  var arr = [3, , 1, , 2];
  var sorted = arr.toSorted();
  print(sorted.length);
  // CHECK-NEXT: 5
  print(sorted[0], sorted[1], sorted[2], sorted[3], sorted[4]);
  // CHECK-NEXT: 1 2 3 undefined undefined
})();

// Sparse array-like objects become dense sorted arrays without changing the
// receiver.
(function () {
  var len = 10000;
  var obj = {};
  obj.prop = "prop";
  obj[0] = 100;
  obj[5] = undefined;
  obj[len - 1] = 0;
  obj.length = len;

  var sorted = Array.prototype.toSorted.call(obj);
  print(sorted.length);
  // CHECK-NEXT: 10000
  print(sorted[0], sorted[1], sorted[2], sorted[len - 1]);
  // CHECK-NEXT: 0 100 undefined undefined
  print(Object.prototype.hasOwnProperty.call(sorted, 2));
  // CHECK-NEXT: true
  print(obj[0], obj[5], obj[len - 1], obj.prop, obj.length);
  // CHECK-NEXT: 100 undefined 0 prop 10000
})();

// Array-like objects via .call.
(function () {
  var obj = {0: "banana", 1: "apple", 2: "cherry", length: 3};
  var sorted = Array.prototype.toSorted.call(obj);
  print(sorted.join(","));
  // CHECK-NEXT: apple,banana,cherry
  print(Array.isArray(sorted));
  // CHECK-NEXT: true
})();

// TypeError for non-callable comparefn.
(function () {
  try {
    [1, 2].toSorted(42);
    print("FAIL: no error");
  } catch (e) {
    print(e.constructor.name);
  }
  // CHECK-NEXT: TypeError
})();

// comparefn returning NaN is treated as +0.
(function () {
  var arr = [3, 1, 2];
  var sorted = arr.toSorted(function () { return NaN; });
  print(sorted.join(","));
  // CHECK-NEXT: 3,1,2
  print(arr.join(","));
  // CHECK-NEXT: 3,1,2
})();

// comparefn that mutates the original array does not affect the result.
(function () {
  var arr = [5, 3, 1, 4, 2];
  var sorted = arr.toSorted(function (a, b) {
    arr.length = 0;
    return a - b;
  });
  print(sorted.join(","));
  // CHECK-NEXT: 1,2,3,4,5
  print(arr.length);
  // CHECK-NEXT: 0
})();

// Function.length and Function.name.
(function () {
  print(Array.prototype.toSorted.length);
  // CHECK-NEXT: 1
  print(Array.prototype.toSorted.name);
  // CHECK-NEXT: toSorted
})();
