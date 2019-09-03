// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

var e = new Error();

var e2 = new Error();
e2.name = 'RandomError';
e2.message = 'random-message';

var e3 = new TypeError();

serializeVM(function() {
  print(e);
  //CHECK: Error

  print(e.hasOwnProperty(e, 'name'));
  //CHECK: false
  print(e.hasOwnProperty(e, 'message'));
  //CHECK: false
  print(e.__proto__.hasOwnProperty('name'));
  //CHECK: true
  print(e.__proto__.hasOwnProperty('message'));
  //CHECK: true

  e2.name = 'RandomError';
  e2.message = 'random-message';
  print(e2);
  //CHECK: RandomError: random-message

  print(e3);
  //CHECK: TypeError

  print(e3.hasOwnProperty(e, 'name'));
  //CHECK: false
  print(e3.hasOwnProperty(e, 'message'));
  //CHECK: false
  print(e3.__proto__.hasOwnProperty('name'));
  //CHECK: true
  print(e3.__proto__.hasOwnProperty('message'));
  //CHECK: true
})
