// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

m = new WeakMap()
var x = {}
var y = {}
var m1 = new WeakMap(new Set([[x,2], [y,4]]));

var a = {};
var b = {x:'abc'};
var c = {y:3};
var d = {x:1}
var m2 = new WeakMap([[a, 0x123], [b, 0xbeef]]);

serializeVM(function() {
  print("WeakMap");
  // CHECK-LABEL: WeakMap
  print(m);
  // CHECK-NEXT: [object WeakMap]
  print(m1.get(x), m1.get(y));
  // CHECK-NEXT: 2 4

  print(m2.get(a), m2.get(b), m2.get(c));
  // CHECK-NEXT: 291 48879 undefined
  print(m2.has(a), m2.has(b), m2.has(c));
  // CHECK-NEXT: true true false
  // see if we can still set after deserialization.
  m2.set(a, 12);
  print(m2.get(a));
  // CHECK-NEXT: 12
  print(m2.delete(a), m2.delete(d));
  // CHECK-NEXT: true false
})
