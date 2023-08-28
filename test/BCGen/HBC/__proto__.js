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
//CHECK-LABEL:Function<staticProto>(1 params, 11 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    NewObjectWithBuffer r0, 3, 3, 0, 0
//CHECK-NEXT:    LoadConstNull     r2
//CHECK-NEXT:    Mov               r3, r0
//CHECK-NEXT:    CallBuiltin       r1, "HermesBuiltin.silentSetPrototypeOf", 3
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

function sideEffectProto(){
  var g = globalThis;
  g.x = 10;
  g.n = null;
  return {__proto__:  (++g.x, g.n), a: g.x, b: 3, c: 4, d: 5};
}
//CHECK-LABEL:Function<sideEffectProto>(1 params, 12 registers, 0 symbols):
//CHECK-NEXT: Offset in debug table: {{.*}}
//CHECK-NEXT:     GetGlobalObject   r0
//CHECK-NEXT:     TryGetById        r1, r0, 1, "globalThis"
//CHECK-NEXT:     LoadConstUInt8    r0, 10
//CHECK-NEXT:     PutById           r1, r0, 1, "x"
//CHECK-NEXT:     LoadConstNull     r0
//CHECK-NEXT:     PutById           r1, r0, 2, "n"
//CHECK-NEXT:     GetByIdShort      r0, r1, 2, "x"
//CHECK-NEXT:     Inc               r0, r0
//CHECK-NEXT:     PutById           r1, r0, 1, "x"
//CHECK-NEXT:     GetByIdShort      r3, r1, 3, "n"
//CHECK-NEXT:     NewObjectWithBuffer r0, 4, 4, 4, 13
//CHECK-NEXT:     Mov               r4, r0
//CHECK-NEXT:     CallBuiltin       r2, "HermesBuiltin.silentSetPrototypeOf", 3
//CHECK-NEXT:     GetByIdShort      r1, r1, 2, "x"
//CHECK-NEXT:     PutById           r0, r1, 3, "a"
//CHECK-NEXT:     Ret               r0
