// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

var re = new RegExp(new RegExp(), "m");
var re2 = new RegExp(new RegExp("hello", "g"))
var re3 = new RegExp(/abc/g, "m")
var re4 = new RegExp("abc", "mig")
var re5 = new RegExp(true)
var re6 = new RegExp(undefined)

var re7 = RegExp("\\w (\\d+)")
var m = RegExp("\\w (\\d+)").exec("abc 1234");

var re8 = RegExp("a", "g");

var re9 = RegExp("abc\ndef")

serializeVM(function() {
  print('RegExp');
  // CHECK-LABEL: RegExp
  print(RegExp.prototype.constructor === RegExp);
  // CHECK-NEXT: true
  print(re);
  // CHECK-NEXT: /(?:)/m
  print(re2);
  // CHECK-NEXT: /hello/g
  print(re3);
  // CHECK-NEXT: /abc/m
  print((re4).toString());
  // CHECK-NEXT: /abc/gim
  print(re5);
  // CHECK-NEXT: /true/
  print(re6);
  // CHECK-NEXT: /(?:)/

  print(re7.exec("abc def"));
  // CHECK-NEXT: null
  print(re7.exec("abc 1234"));
  // CHECK-NEXT: c 1234,1234
  print(m.input + ", " + m.index + ", " + m.length + ", " + m[1]);
  // CHECK-NEXT: abc 1234, 2, 2, 1234

  // Check lastIndex handling
  print(re8.lastIndex);
  // CHECK-NEXT: 0
  print(re8.exec("aab"));
  // CHECK-NEXT: a
  print(re8.lastIndex);
  // CHECK-NEXT: 1

  print(re9.source);
  // CHECK-NEXT: abc\ndef
})
