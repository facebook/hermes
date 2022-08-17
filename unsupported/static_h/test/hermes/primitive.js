/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s


print("start of test");
//CHECK: start of test

// Test data property on primitive prototype.
var x = 15;
Number.prototype.myProperty = "foo";
print(x.myProperty);
//CHECK-NEXT: foo

// Test getter accessor property with leaking.
var leak;
Object.defineProperty(Number.prototype, "accessor", {
  get: function() {
    leak = this;
    return "accessor-getter";
  },
});
print(x.accessor);
//CHECK-NEXT: accessor-getter

// Leaked base object should be boxed.
print(typeof(leak));
//CHECK-NEXT: object

print(x.nonExist);
//CHECK-NEXT: undefined

var s = "abc";
String.prototype[9] = "bar";

// Test primitive own property.
print(s.length);
//CHECK-NEXT: 3

// Test normal array index
print(s[1]);
//CHECK-NEXT: b

// Test array index property in prototype.
print(s[9]);
//CHECK-NEXT: bar

// Test property key toString() failure.
try {
  var obj = {toString: function() { throw "exception thrown!"; }};
  print(s[obj]);
} catch (e) {
  print("caught exception");
  print(e)
}
//CHECK-NEXT: caught exception
//CHECK-NEXT: exception thrown!

// Test computed property of primitive own property.
function getPropName() {
  return "length";
}
print(s[getPropName()]);
//CHECK-NEXT: 3

// Test data property on undefined.
try {
  print(undefined.length);
} catch(e) {print('caught', e.name, e.message);}
// CHECK-NEXT: caught TypeError {{.*}}
