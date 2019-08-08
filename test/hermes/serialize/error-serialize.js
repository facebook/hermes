// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -serialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -deserialize-file=%t.sd.dat -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

// Test Error
var e = new Error();
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

e.name = 'RandomError';
print(e);
//CHECK: RandomError

e.message = 'random-message';
print(e);
//CHECK: RandomError: random-message

// Test Native error types
print(EvalError.__proto__ === Error);
//CHECK: true
print(RangeError.__proto__ === Error);
//CHECK: true
print(ReferenceError.__proto__ === Error);
//CHECK: true
print(SyntaxError.__proto__ === Error);
//CHECK: true
print(TypeError.__proto__ === Error);
//CHECK: true
print(URIError.__proto__ === Error);
//CHECK: true

// Check exception case of accessing Error.stack from a different 'this'
try {
  new Error().__lookupGetter__("stack").call({});
} catch (e) {
  print(e.name);
}
//CHECK: TypeError
