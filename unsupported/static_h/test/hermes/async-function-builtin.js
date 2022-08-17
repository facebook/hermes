/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

print('AsyncFunction instance');
// CHECK-LABEL: AsyncFunction instance

async function af(){};
print(Reflect.ownKeys(af));
// CHECK-NEXT: length,name
print(af.prototype);
// CHECK-NEXT: undefined
try {
  new af();
} catch (e){
  print(e);
}
// CHECK-NEXT: TypeError: Function is not a constructor

print('AsyncFunction prototype object');
// CHECK-LABEL: AsyncFunction prototype object

var AFP = af.__proto__;
print(AFP[Symbol.toStringTag])
// CHECK-NEXT: AsyncFunction

// AsyncFunction constructor
print('AsyncFunction constructor');
// CHECK-LABEL: AsyncFunction constructor

var AF = af.__proto__.constructor;
print(Reflect.ownKeys(AF));
// CHECK-NEXT: length,name,prototype
print(AF.name);
// CHECK-NEXT: AsyncFunction
print(AF.length);
// CHECK-NEXT: 1
print(AF.prototype === AFP);
// CHECK-NEXT: true
print(AF() instanceof AF);
// CHECK-NEXT: true
print(new AF() instanceof AF);
// CHECK-NEXT: true
print(AF.__proto__ == Function);
// CHECK-NEXT: true
