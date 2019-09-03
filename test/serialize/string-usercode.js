// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";

var empty = new String()
var s = new String('asdf')
var s2 = new String(133)
var s3 = new String(undefined)

serializeVM(function() {
  print('String');
  // CHECK-LABEL: String
  print('empty', empty);
  // CHECK-NEXT: empty
  print(s, s.length);
  // CHECK-NEXT: asdf 4
  print(s2);
  // CHECK-NEXT: 133
  print(s3);
  // CHECK-NEXT: undefined
  print(s, s.toString(), s.valueOf(), s.length, s.__proto__ === String.prototype);
  // CHECK-NEXT: asdf asdf asdf 4 true
  var desc = Object.getOwnPropertyDescriptor(s, 'length');
  print(desc.enumerable, desc.writable, desc.configurable);
  // CHECK-NEXT: false false false
})
