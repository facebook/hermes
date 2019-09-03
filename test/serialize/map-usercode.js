// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

var m = new Map()
var m1 = new Map([]);
var m2 = new Map([[1,2]]);
var m3 = new Map([[1,2], [3,4]]);

function callbackFn(value, key, s) {
  print(key, value, s.size);
}
s = new Map();
s.set(1, []);
s.set({}, -0);
s.set(undefined, undefined);

serializeVM(function() {
  var m = new Map()
  print(m);
  //CHECK: [object Map]
  var m1 = new Map([]);
  print(m1.size);
  // CHECK-NEXT: 0
  var m2 = new Map([[1,2]]);
  print(m2.size, m2.get(1));
  // CHECK-NEXT: 1 2
  var m3 = new Map([[1,2], [3,4]]);
  print(m3.size, m3.get(1), m3.get(3));
  // CHECK-NEXT: 2 2 4

  s.forEach(callbackFn);
  //CHECK-NEXT: 1  3
  //CHECK-NEXT: [object Object] 0 3
  //CHECK-NEXT: undefined undefined 3
  s.set("abc", "");
  s.forEach(callbackFn);
  //CHECK-NEXT: 1  4
  //CHECK-NEXT: [object Object] 0 4
  //CHECK-NEXT: undefined undefined 4
  //CHECK-NEXT: abc  4
})
