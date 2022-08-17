/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

print("instanceof");
//CHECK-LABEL: instanceof

try {
    1 instanceof 2;
} catch (e) {
    print("caught", e.name, e.message);
}
//CHECK-NEXT: caught TypeError right operand of 'instanceof' is not an object

function foo () {}
foo.prototype = 1;

print(1 instanceof foo);
//CHECK-NEXT: false

try {
    ({}) instanceof foo;
} catch (e) {
    print("caught", e.name, e.message);
}
//CHECK-NEXT: caught TypeError function's '.prototype' is not an object in 'instanceof'

function BaseObj() {}

print({} instanceof BaseObj);
//CHECK-NEXT: false

print( (new BaseObj()) instanceof BaseObj);
//CHECK-NEXT: true

function ChildObj() {}
ChildObj.prototype = Object.create(BaseObj.prototype);

print( (new ChildObj()) instanceof BaseObj);
//CHECK-NEXT: true
print( (new ChildObj()) instanceof ChildObj);
//CHECK-NEXT: true

var BoundBase = BaseObj.bind(1,2).bind(3,4);
var BoundChild = ChildObj.bind(1,2).bind(3,4);

print( (new ChildObj()) instanceof BoundBase);
//CHECK-NEXT: true
print( (new ChildObj()) instanceof BoundChild);
//CHECK-NEXT: true

var a = new Proxy({}, {});
var b = Object.create(a);
var c = Object.create(b);
a.__proto__ = c;
try {
    b instanceof Date;
} catch (e) {
    print("caught", e.name, e.message);
}
//CHECK-NEXT: caught RangeError Maximum prototype chain length exceeded

