// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict"

var cons = [
  Int8Array,
  Int16Array,
  Int32Array,
  Uint8Array,
  Uint8ClampedArray,
  Uint16Array,
  Uint32Array,
  Float32Array,
  Float64Array,
];

var arrays = new Array(cons.length);
var i;
for (i = 0; i < arrays.length; i++) {
  arrays[i] = new cons[i](new ArrayBuffer(32));
}

var arrays2 = new Array(cons.length);
for (i = 0; i < arrays2.length; i++) {
  arrays2[i] = new cons[i](new ArrayBuffer(32));
  arrays2[i][0] = 1;
  arrays2[i][1] = 2;
}

var arrays3 = new Array(cons.length);
for (i = 0; i < arrays3.length; i++) {
  arrays3[i] = new cons[i](new ArrayBuffer(32), 8, 1);
}

serializeVM(function() {
  for (i = 0; i < arrays.length; i++) {
    print(arrays[i].length);
  }
  // CHECK: 32
  // CHECK-NEXT: 16
  // CHECK-NEXT: 8
  // CHECK-NEXT: 32
  // CHECK-NEXT: 32
  // CHECK-NEXT: 16
  // CHECK-NEXT: 8
  // CHECK-NEXT: 8
  // CHECK-NEXT: 4

  for (i = 0; i < arrays2.length; i++) {
    print(arrays2[i]);
  }
  // CHECK-NEXT: 1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  // CHECK-NEXT: 1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  // CHECK-NEXT: 1,2,0,0,0,0,0,0
  // CHECK-NEXT: 1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  // CHECK-NEXT: 1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  // CHECK-NEXT: 1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  // CHECK-NEXT: 1,2,0,0,0,0,0,0
  // CHECK-NEXT: 1,2,0,0,0,0,0,0
  // CHECK-NEXT: 1,2,0,0

  for (i = 0; i < arrays2.length; i++) {
    print(arrays3[i].byteOffset);
  }
  // CHECK-NEXT: 8
  // CHECK-NEXT: 8
  // CHECK-NEXT: 8
  // CHECK-NEXT: 8
  // CHECK-NEXT: 8
  // CHECK-NEXT: 8
  // CHECK-NEXT: 8
  // CHECK-NEXT: 8
  // CHECK-NEXT: 8
})
