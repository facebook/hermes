// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict";

var n = new Number(123)
var n2 = new Number(Infinity)
var n3 = new Number("123asdf")
var n4 = new Number(123000);
var n5 = new Number(2.2250738585072e-308)

serializeVM(function() {
  print('Number');
  // CHECK-LABEL: Number
  print(n)
  // CHECK-NEXT: 123
  print(n2)
  // CHECK-NEXT: Infinity
  print(n3)
  // CHECK-NEXT: NaN
  print(n4, n4.__proto__ === Number.prototype);
  // CHECK-NEXT: 123000 true
  var s = n5.toString(36)
  print(s.length)
  // CHECK-NEXT: 209
  print(s.slice(190));
  // CHECK-NEXT: 00000000034lmua2oev
  print(Array.prototype.every.call(
    s.slice(2, 190),
    function(c) {return c === '0'}))
  // CHECK-NEXT: true
})
