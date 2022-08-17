/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var obj = Object();
var proto = Object();
proto.prop1 = "Property1";

print(obj.prop1, proto.prop1);
//CHECK: undefined Property1
print(obj.__proto__ == Object.prototype, obj.__proto__ == Object.getPrototypeOf(obj));
//CHECK-NEXT: true true

obj.__proto__ = proto;
print(obj.prop1);
//CHECK-NEXT: Property1
print(obj.__proto__ == proto, Object.getPrototypeOf(obj) === proto);
//CHECK-NEXT: true true

// Setting the prototype to something other than object or null must be ignored
obj.__proto__ = 10;
print(obj.__proto__ == proto);
//CHECK-NEXT: true

obj.__proto__ = Object();
print(obj.prop1);
//CHECK-NEXT: undefined

// Ensure that missing prototype is returned as null
print({}.__proto__.__proto__, Object.getPrototypeOf({}.__proto__));
//CHECK-NEXT: null null

try {
  var x = Object.preventExtensions({});
  x.__proto__ = {};
} catch (e) {
  print(e.name);
}
//CHECK: TypeError

