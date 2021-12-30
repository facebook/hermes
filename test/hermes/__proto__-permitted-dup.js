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

o = {__proto__: proto1, a: 10, __proto__(x,y) { return x + y;}}
print(Object.getPrototypeOf(o) === proto1);
print(o.__proto__(35,7));
//CHKBC: NewObjectWithParent r
//CHKBC-NOT: CallBuiltin
//CHECK: true
//CHECK-NEXT: 42

o = {__proto__(x,y) { return x + y;}, __proto__: proto1, a: 10}
print(Object.getPrototypeOf(o) === proto1);
print(o.__proto__(35,8));
//CHKBC: NewObject r
//CHKBC: CallBuiltin
//CHECK-NEXT: true
//CHECK-NEXT: 43
