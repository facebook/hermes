// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

var s = new WeakSet()
var x = {}
var y = {}
var s1 = new WeakSet(new Set([x]));

var a = {};
var b = {x:'abc'};
var c = {y:3};
var m = new WeakSet([a,b]);

serializeVM(function() {
  print("WeakSet");
  // CHECK-LABEL: WeakSet
  print(s);
  // CHECK-NEXT: [object WeakSet]

  print(s1.has(x), s1.has(y));
  // CHECK-NEXT: true false

  print('has');
  // CHECK-LABEL: has
  print(m.has(a), m.has(b), m.has(c));
  // CHECK-NEXT: true true false

  // see if we can still add/delete after deserialization.
  print(s.has(a), s.has(b));
  // CHECK-NEXT: false false
  s.add(a);
  print(s.has(a), s.has(b));
  // CHECK-NEXT: true false
  s.add(b);
  print(s.has(a), s.has(b));
  // CHECK-NEXT: true true
  print(s.delete(a), s.delete(b), s.delete(c));
  // CHECK-NEXT: true true false
})
