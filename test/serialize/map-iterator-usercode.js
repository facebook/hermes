// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

s = new Map();
s.set(1, {});
s.set({}, 1);
s.set(undefined, null);
s.set("abc", "def");

var keys = s.keys();
var values = s.values();
var entries = s.entries();

serializeVM(function() {
  print("testIteration");
  //CHECK-LABEL: testIteration
  print(keys.__proto__);
  //CHECK-NEXT: [object Map Iterator]
  print(keys[Symbol.iterator]());
  //CHECK-NEXT: [object Map Iterator]
  print(Object.getOwnPropertyNames(keys.__proto__));
  //CHECK-NEXT: next
  while (true) {
    var e = keys.next();
    print(e.value);
    if (e.done) break;
  }
  //CHECK-NEXT: 1
  //CHECK-NEXT: [object Object]
  //CHECK-NEXT: undefined
  //CHECK-NEXT: abc
  //CHECK-NEXT: undefined

  while (true) {
    var e = values.next();
    print(e.value);
    if (e.done) break;
  }
  //CHECK-NEXT: [object Object]
  //CHECK-NEXT: 1
  //CHECK-NEXT: null
  //CHECK-NEXT: def
  //CHECK-NEXT: undefined

  while (true) {
    var e = entries.next();
    print(e.value);
    if (e.done) break;
  }
  //CHECK-NEXT: 1,[object Object]
  //CHECK-NEXT: [object Object],1
  //CHECK-NEXT: ,
  //CHECK-NEXT: abc,def
  //CHECK-NEXT: undefined
})
