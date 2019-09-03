// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";

// Runs an iterator until it is done.
// Prints out `value` and `done` every iteration.
function runIterator(it) {
  print(it.toString());
  while (true) {
    var result = it.next();
    print(result.value, result.done);
    if (result.done) return;
  }
}

var a = ['a','b','c'];
var ai1 = a.keys();
var ai2 = a.values();
var ai3 = a.entries();

var a2 = [];
var ai4 = a2.keys();
var ai5 = a2.values();
var ai6 = a2.entries();

var a3 = new Uint8Array([10, 11, 12]);
var ti1 = a3.keys();
var ti2 = a3.values();
var ti3 = a3.entries();

var a4 = new Uint8Array([]);
var ti4 = a4.keys();
var ti5 = a4.values();
var ti6 = a4.entries();

var a5 = 'abcd';
var si1 = a5[Symbol.iterator]();
var a6 = '';
var si2 = a6[Symbol.iterator]();
var a7 = 'x\uD83D\uDCD3y';
var si3 = a7[Symbol.iterator]();

serializeVM(function() {
  print('Array Iterator');
  // CHECK-LABEL: Array Iterator
  runIterator(ai1);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: 0 false
  // CHECK-NEXT: 1 false
  // CHECK-NEXT: 2 false
  // CHECK-NEXT: undefined true
  runIterator(ai2);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: a false
  // CHECK-NEXT: b false
  // CHECK-NEXT: c false
  // CHECK-NEXT: undefined true
  runIterator(ai3);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: 0,a false
  // CHECK-NEXT: 1,b false
  // CHECK-NEXT: 2,c false
  // CHECK-NEXT: undefined true
  runIterator(ai4);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: undefined true
  runIterator(ai5);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: undefined true
  runIterator(ai6);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: undefined true

  print('TypedArray Iterator');
  // CHECK-LABEL: TypedArray Iterator
  runIterator(ti1);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: 0 false
  // CHECK-NEXT: 1 false
  // CHECK-NEXT: 2 false
  // CHECK-NEXT: undefined true
  runIterator(ti2);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: 10 false
  // CHECK-NEXT: 11 false
  // CHECK-NEXT: 12 false
  // CHECK-NEXT: undefined true
  runIterator(ti3);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: 0,10 false
  // CHECK-NEXT: 1,11 false
  // CHECK-NEXT: 2,12 false
  // CHECK-NEXT: undefined true
  runIterator(ti4);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: undefined true
  runIterator(ti5);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: undefined true
  runIterator(ti6);
  // CHECK-NEXT: [object Array Iterator]
  // CHECK-NEXT: undefined true

  print('String Iterator');
  // CHECK-LABEL: String Iterator
  runIterator(si1);
  // CHECK-NEXT: [object String Iterator]
  // CHECK-NEXT: a false
  // CHECK-NEXT: b false
  // CHECK-NEXT: c false
  // CHECK-NEXT: d false
  // CHECK-NEXT: undefined true
  runIterator(si2);
  // CHECK-NEXT: [object String Iterator]
  // CHECK-NEXT: undefined true
  runIterator(si3);
  // CHECK-NEXT: [object String Iterator]
  // CHECK-NEXT: x false
  // CHECK-NEXT: 📓 false
  // CHECK-NEXT: y false
  // CHECK-NEXT: undefined true
})
