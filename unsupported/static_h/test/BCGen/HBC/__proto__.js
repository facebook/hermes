/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-bytecode %s | %FileCheck --match-full-lines %s

// Code generation for static proto
function staticProto() {
  return {__proto__: null, a: 2, b: 3, c: 4};
}
//CHECK-LABEL:Function<staticProto>(1 params, 2 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
//CHECK-NEXT:    LoadConstNull     r0
//CHECK-NEXT:    NewObjectWithParent r0, r0
//CHECK-NEXT:    LoadConstUInt8    r1, 2
//CHECK-NEXT:    PutNewOwnByIdShort r0, r1, "a"
//CHECK-NEXT:    LoadConstUInt8    r1, 3
//CHECK-NEXT:    PutNewOwnByIdShort r0, r1, "b"
//CHECK-NEXT:    LoadConstUInt8    r1, 4
//CHECK-NEXT:    PutNewOwnByIdShort r0, r1, "c"
//CHECK-NEXT:    Ret               r0

function dynamicProto(func, getProto) {
  return {a: func(), b: 10, __proto__: getProto()};
}
//CHECK-LABEL:Function<dynamicProto>(3 params, 12 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    NewObject         r0
//CHECK-NEXT:    LoadParam         r1, 1
//CHECK-NEXT:    LoadConstUndefined r2
//CHECK-NEXT:    Call1             r1, r1, r2
//CHECK-NEXT:    PutNewOwnByIdShort r0, r1, "a"
//CHECK-NEXT:    LoadConstUInt8    r1, 10
//CHECK-NEXT:    PutNewOwnByIdShort r0, r1, "b"
//CHECK-NEXT:    LoadParam         r1, 2
//CHECK-NEXT:    Call1             r3, r1, r2
//CHECK-NEXT:    Mov               r4, r0
//CHECK-NEXT:    CallBuiltin       r1, "HermesBuiltin.silentSetPrototypeOf", 3
//CHECK-NEXT:    Ret               r0
