/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

print("define-property");
//CHECK-LABEL: define-property

// Check sure that the accessor flag is not cleared when updating other flags.
var obj =  { get p() { return 42; }  };
Object.defineProperty(obj, 'p', {enumerable: false });
print(obj.p);
//CHECK-NEXT: 42

// Check converting from accessor to data property by setting writable to false
obj = {};
Object.defineProperty(obj, "prop", {
    get:function() {return 10;},
    set: undefined,
    enumerable: false,
    configurable: true
});
var desc = Object.getOwnPropertyDescriptor(obj, 'prop');
print(desc.writable, typeof desc.get);
//CHECK-NEXT: undefined function
print(obj.prop);
//CHECK-NEXT: 10

Object.defineProperty(obj, 'prop', { writable: false, configurable: false });
desc = Object.getOwnPropertyDescriptor(obj, 'prop');
print(desc.writable, typeof desc.get);
//CHECK-NEXT: false undefined
print(obj.prop);
//CHECK-NEXT: undefined

// Verify that key is converted to primitive before desc object is parsed
var key = { [Symbol.toPrimitive]: function() { throw "badkey" } };
try {
  Object.defineProperty(obj, key, undefined);
  print("Succeeded");
} catch (e) {
  print("Caught", e);
}
//CHECK-NEXT: Caught badkey
