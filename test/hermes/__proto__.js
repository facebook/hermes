/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %hermes -O -dump-bytecode %s | %FileCheck --check-prefix=CHKBC %s

var proto1 = Object()
var proto2 = Object()

function getProtoX(x) {
    return x;
}

var o;

o = {__proto__: proto1, a: 10}
print(Object.getPrototypeOf(o) === proto1);
//CHKBC: NewObjectWithParent r
//CHKBC-NOT: CallBuiltin
//CHECK: true

o = {__proto__: null, a: 10}
print(Object.getPrototypeOf(o) === null);
//CHKBC: NewObjectWithParent r
//CHKBC-NOT: CallBuiltin
//CHECK-NEXT: true

o = {__proto__: 10, a: 10}
print(Object.getPrototypeOf(o) === Object.prototype);
//CHKBC: NewObjectWithParent r
//CHKBC-NOT: CallBuiltin
//CHECK-NEXT: true

o = {__proto__: undefined, a: 10}
print(Object.getPrototypeOf(o) === Object.prototype);
//CHKBC: NewObjectWithParent r
//CHKBC-NOT: CallBuiltin
//CHECK-NEXT: true

o = {__proto__: "proto", a: 10}
print(Object.getPrototypeOf(o) === Object.prototype);
//CHKBC: NewObjectWithParent r
//CHKBC-NOT: CallBuiltin
//CHECK-NEXT: true

o = {b: 20, __proto__: getProtoX(proto2)}
print(Object.getPrototypeOf(o) === proto2);
//CHKBC: NewObject r
//CHKBC: CallBuiltin
//CHECK-NEXT: true

o = {b: 20, __proto__: getProtoX(null)}
print(Object.getPrototypeOf(o) === null);
//CHKBC: NewObject r
//CHKBC: CallBuiltin
//CHECK-NEXT: true

o = {b: 20, __proto__: getProtoX(20)}
print(Object.getPrototypeOf(o) === Object.prototype);
//CHKBC: NewObject r
//CHKBC: CallBuiltin
//CHECK-NEXT: true

o = {b: 20, __proto__: getProtoX(undefined)}
print(Object.getPrototypeOf(o) === Object.prototype);
//CHKBC: NewObject r
//CHKBC: CallBuiltin
//CHECK-NEXT: true

o = {b: 20, __proto__: getProtoX("proto")}
print(Object.getPrototypeOf(o) === Object.prototype);
//CHKBC: NewObject r
//CHKBC: CallBuiltin
//CHECK-NEXT: true
