/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheckOrRegen %s --match-full-lines

async function simpleReturn() {
  return 1;
}

async function simpleAwait() {
  var x = await 2;
  return x;
}

var simpleAsyncFE = async function () {
  var x = await 2;
  return x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 10
// CHECK-NEXT:  String count: 14
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 3
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..-1]:
// CHECK-NEXT:s1[ASCII, 0..28]: ?anon_0_?anon_0_simpleAsyncFE
// CHECK-NEXT:s2[ASCII, 29..55]: ?anon_0_?anon_0_simpleAwait
// CHECK-NEXT:s3[ASCII, 56..83]: ?anon_0_?anon_0_simpleReturn
// CHECK-NEXT:s4[ASCII, 84..104]: ?anon_0_simpleAsyncFE
// CHECK-NEXT:s5[ASCII, 105..123]: ?anon_0_simpleAwait
// CHECK-NEXT:s6[ASCII, 124..143]: ?anon_0_simpleReturn
// CHECK-NEXT:s7[ASCII, 144..204]: Generator functions may not be called on executing generators
// CHECK-NEXT:s8[ASCII, 205..210]: global
// CHECK-NEXT:i9[ASCII, 211..214] #B553E7BD: done
// CHECK-NEXT:i10[ASCII, 215..227] #4CCB9499: simpleAsyncFE
// CHECK-NEXT:i11[ASCII, 228..238] #FD482E4F: simpleAwait
// CHECK-NEXT:i12[ASCII, 239..250] #EB416734: simpleReturn
// CHECK-NEXT:i13[ASCII, 251..255] #DF50693D: value

// CHECK:Literal Value Buffer:
// CHECK-NEXT:[int 1]
// CHECK-NEXT:true
// CHECK-NEXT:null
// CHECK-NEXT:true
// CHECK-NEXT:[int 2]
// CHECK-NEXT:false
// CHECK-NEXT:Object Key Buffer:
// CHECK-NEXT:[String 13]
// CHECK-NEXT:[String 9]
// CHECK-NEXT:Function Source Table:
// CHECK-NEXT:  Function ID 7 -> s0
// CHECK-NEXT:  Function ID 8 -> s0
// CHECK-NEXT:  Function ID 9 -> s0

// CHECK:Function<global>(1 params, 11 registers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r0, 0
// CHECK-NEXT:    DeclareGlobalVar  "simpleReturn"
// CHECK-NEXT:    DeclareGlobalVar  "simpleAwait"
// CHECK-NEXT:    DeclareGlobalVar  "simpleAsyncFE"
// CHECK-NEXT:    CreateClosure     r1, r0, NCFunction<simpleReturn>
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "simpleReturn"
// CHECK-NEXT:    CreateClosure     r3, r0, NCFunction<simpleAwait>
// CHECK-NEXT:    GetGlobalObject   r4
// CHECK-NEXT:    PutByIdLoose      r4, r3, 2, "simpleAwait"
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    Mov               r5, r6
// CHECK-NEXT:    CreateClosure     r7, r0, NCFunction<simpleAsyncFE>
// CHECK-NEXT:    GetGlobalObject   r8
// CHECK-NEXT:    PutByIdLoose      r8, r7, 3, "simpleAsyncFE"
// CHECK-NEXT:    Mov               r9, r5
// CHECK-NEXT:    Ret               r9

// CHECK:NCFunction<simpleReturn>(1 params, 23 registers):
// CHECK-NEXT:Offset in debug table: source 0x0016, lexical 0x0000
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Mov               r0, r1
// CHECK-NEXT:    LoadThisNS        r2
// CHECK-NEXT:    GetParentEnvironment r3, 0
// CHECK-NEXT:    CreateEnvironment r4, r3, 0
// CHECK-NEXT:    CreateClosure     r5, r4, NCFunction<?anon_0_simpleReturn>
// CHECK-NEXT:    GetBuiltinClosure r6, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArgumentsLoose r0
// CHECK-NEXT:    Mov               r7, r0
// CHECK-NEXT:    LoadConstUndefined r8
// CHECK-NEXT:    LoadConstUndefined r9
// CHECK-NEXT:    Call4             r10, r6, r9, r5, r2, r7
// CHECK-NEXT:    Ret               r10

// CHECK:NCFunction<simpleAwait>(1 params, 23 registers):
// CHECK-NEXT:Offset in debug table: source 0x001d, lexical 0x0000
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Mov               r0, r1
// CHECK-NEXT:    LoadThisNS        r2
// CHECK-NEXT:    GetParentEnvironment r3, 0
// CHECK-NEXT:    CreateEnvironment r4, r3, 0
// CHECK-NEXT:    CreateClosure     r5, r4, NCFunction<?anon_0_simpleAwait>
// CHECK-NEXT:    GetBuiltinClosure r6, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArgumentsLoose r0
// CHECK-NEXT:    Mov               r7, r0
// CHECK-NEXT:    LoadConstUndefined r8
// CHECK-NEXT:    LoadConstUndefined r9
// CHECK-NEXT:    Call4             r10, r6, r9, r5, r2, r7
// CHECK-NEXT:    Ret               r10

// CHECK:NCFunction<simpleAsyncFE>(1 params, 23 registers):
// CHECK-NEXT:Offset in debug table: source 0x0024, lexical 0x0000
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Mov               r0, r1
// CHECK-NEXT:    LoadThisNS        r2
// CHECK-NEXT:    GetParentEnvironment r3, 0
// CHECK-NEXT:    CreateEnvironment r4, r3, 0
// CHECK-NEXT:    CreateClosure     r5, r4, NCFunction<?anon_0_simpleAsyncFE>
// CHECK-NEXT:    GetBuiltinClosure r6, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArgumentsLoose r0
// CHECK-NEXT:    Mov               r7, r0
// CHECK-NEXT:    LoadConstUndefined r8
// CHECK-NEXT:    LoadConstUndefined r9
// CHECK-NEXT:    Call4             r10, r6, r9, r5, r2, r7
// CHECK-NEXT:    Ret               r10

// CHECK:NCFunction<?anon_0_simpleReturn>(1 params, 7 registers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    CreateEnvironment r1, r0, 5
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadConstZero     r3
// CHECK-NEXT:    StoreNPToEnvironment r1, 4, r3
// CHECK-NEXT:    LoadConstZero     r4
// CHECK-NEXT:    StoreNPToEnvironment r1, 2, r4
// CHECK-NEXT:    CreateGenerator   r5, r1, Function<?anon_0_?anon_0_simpleReturn>
// CHECK-NEXT:    Ret               r5

// CHECK:NCFunction<?anon_0_simpleAwait>(1 params, 7 registers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    CreateEnvironment r1, r0, 7
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadConstZero     r3
// CHECK-NEXT:    StoreNPToEnvironment r1, 5, r3
// CHECK-NEXT:    LoadConstZero     r4
// CHECK-NEXT:    StoreNPToEnvironment r1, 3, r4
// CHECK-NEXT:    CreateGenerator   r5, r1, Function<?anon_0_?anon_0_simpleAwait>
// CHECK-NEXT:    Ret               r5

// CHECK:NCFunction<?anon_0_simpleAsyncFE>(1 params, 7 registers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    CreateEnvironment r1, r0, 7
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadConstZero     r3
// CHECK-NEXT:    StoreNPToEnvironment r1, 5, r3
// CHECK-NEXT:    LoadConstZero     r4
// CHECK-NEXT:    StoreNPToEnvironment r1, 3, r4
// CHECK-NEXT:    CreateGenerator   r5, r1, Function<?anon_0_?anon_0_simpleAsyncFE>
// CHECK-NEXT:    Ret               r5

// CHECK:Function<?anon_0_?anon_0_simpleReturn>(1 params, 19 registers):
// CHECK-NEXT:    LoadParam         r0, 2
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 2
// CHECK-NEXT:    LoadConstUInt8    r4, 2
// CHECK-NEXT:    JStrictEqualLong  L1, r3, r4
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 2
// CHECK-NEXT:    LoadConstUInt8    r4, 3
// CHECK-NEXT:    JStrictEqual      L2, r3, r4
// CHECK-NEXT:    LoadConstUInt8    r3, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r3
// CHECK-NEXT:    LoadFromEnvironment r4, r2, 4
// CHECK-NEXT:    LoadFromEnvironment r4, r2, 1
// CHECK-NEXT:    Mov               r3, r4
// CHECK-NEXT:    LoadConstUInt8    r5, 1
// CHECK-NEXT:    JStrictEqual      L3, r1, r5
// CHECK-NEXT:    StoreToEnvironment r2, 3, r0
// CHECK-NEXT:    LoadFromEnvironment r4, r2, 3
// CHECK-NEXT:    LoadConstUInt8    r5, 2
// CHECK-NEXT:    StrictEq          r6, r1, r5
// CHECK-NEXT:    Mov               r3, r6
// CHECK-NEXT:    Mov               r7, r3
// CHECK-NEXT:    StoreNPToEnvironment r2, 1, r7
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 1
// CHECK-NEXT:    JmpTrue           L4, r8
// CHECK-NEXT:    GetEnvironment    r5, r2, 1
// CHECK-NEXT:    CreateEnvironment r6, r5, 0
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r6
// CHECK-NEXT:    NewObjectWithBuffer r7, 2, 2, 0, 0
// CHECK-NEXT:    Ret               r7
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r5, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r5
// CHECK-NEXT:    NewObjectWithBuffer r6, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r6, r4, 0
// CHECK-NEXT:    Ret               r6
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadConstUInt8    r5, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r5
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUInt8    r5, 1
// CHECK-NEXT:    JStrictEqual      L5, r1, r5
// CHECK-NEXT:    LoadConstUInt8    r5, 2
// CHECK-NEXT:    JStrictEqual      L6, r1, r5
// CHECK-NEXT:    NewObjectWithBuffer r5, 2, 2, 0, 6
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    PutOwnBySlotIdx   r5, r6, 0
// CHECK-NEXT:    Ret               r5
// CHECK-NEXT:L6:
// CHECK-NEXT:    NewObjectWithBuffer r5, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r5, r0, 0
// CHECK-NEXT:    Ret               r5
// CHECK-NEXT:L5:
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r5, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r5
// CHECK-NEXT:    LoadConstString   r6, "Generator functio"...
// CHECK-NEXT:    Mov               r10, r6
// CHECK-NEXT:    CallBuiltin       r7, "HermesBuiltin.throwTypeError", 2
// CHECK-NEXT:    Unreachable

// CHECK:Function<?anon_0_?anon_0_simpleAwait>(1 params, 24 registers):
// CHECK-NEXT:Offset in debug table: source 0x002b, lexical 0x0000
// CHECK-NEXT:    LoadParam         r0, 2
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 3
// CHECK-NEXT:    LoadConstUInt8    r4, 2
// CHECK-NEXT:    JStrictEqualLong  L1, r3, r4
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 3
// CHECK-NEXT:    LoadConstUInt8    r4, 3
// CHECK-NEXT:    JStrictEqualLong  L2, r3, r4
// CHECK-NEXT:    LoadConstUInt8    r3, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r3
// CHECK-NEXT:    LoadFromEnvironment r4, r2, 5
// CHECK-NEXT:    LoadConstZero     r3
// CHECK-NEXT:    StrictEq          r5, r3, r4
// CHECK-NEXT:    JmpTrue           L3, r5
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 2
// CHECK-NEXT:    Mov               r3, r5
// CHECK-NEXT:    LoadConstUInt8    r6, 1
// CHECK-NEXT:    JStrictEqual      L4, r1, r6
// CHECK-NEXT:    StoreToEnvironment r2, 4, r0
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r6, 2
// CHECK-NEXT:    StrictEq          r7, r1, r6
// CHECK-NEXT:    Mov               r3, r7
// CHECK-NEXT:    Mov               r8, r3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r8
// CHECK-NEXT:    LoadFromEnvironment r9, r2, 2
// CHECK-NEXT:    JmpTrue           L5, r9
// CHECK-NEXT:    LoadFromEnvironment r6, r2, 6
// CHECK-NEXT:    StoreToEnvironment r6, 0, r5
// CHECK-NEXT:    LoadFromEnvironment r7, r6, 0
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    NewObjectWithBuffer r9, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r9, r7, 0
// CHECK-NEXT:    Ret               r9
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r6
// CHECK-NEXT:    NewObjectWithBuffer r7, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r7, r5, 0
// CHECK-NEXT:    Ret               r7
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r6
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadFromEnvironment r7, r2, 1
// CHECK-NEXT:    Mov               r6, r7
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    JStrictEqual      L6, r1, r8
// CHECK-NEXT:    StoreToEnvironment r2, 4, r0
// CHECK-NEXT:    LoadFromEnvironment r7, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    StrictEq          r9, r1, r8
// CHECK-NEXT:    Mov               r6, r9
// CHECK-NEXT:    Mov               r10, r6
// CHECK-NEXT:    StoreNPToEnvironment r2, 1, r10
// CHECK-NEXT:    LoadFromEnvironment r11, r2, 1
// CHECK-NEXT:    JmpTrue           L7, r11
// CHECK-NEXT:    GetEnvironment    r8, r2, 1
// CHECK-NEXT:    CreateEnvironment r9, r8, 1
// CHECK-NEXT:    StoreToEnvironment r2, 6, r9
// CHECK-NEXT:    LoadConstUndefined r10
// CHECK-NEXT:    StoreNPToEnvironment r9, 0, r10
// CHECK-NEXT:    LoadConstUInt8    r11, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 5, r11
// CHECK-NEXT:    LoadConstUInt8    r12, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r12
// CHECK-NEXT:    NewObjectWithBuffer r13, 2, 2, 0, 8
// CHECK-NEXT:    Ret               r13
// CHECK-NEXT:L7:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    NewObjectWithBuffer r9, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r9, r7, 0
// CHECK-NEXT:    Ret               r9
// CHECK-NEXT:L6:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    JStrictEqual      L8, r1, r8
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    JStrictEqual      L9, r1, r8
// CHECK-NEXT:    NewObjectWithBuffer r8, 2, 2, 0, 6
// CHECK-NEXT:    LoadConstUndefined r9
// CHECK-NEXT:    PutOwnBySlotIdx   r8, r9, 0
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L9:
// CHECK-NEXT:    NewObjectWithBuffer r8, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r8, r0, 0
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L8:
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    LoadConstString   r9, "Generator functio"...
// CHECK-NEXT:    Mov               r15, r9
// CHECK-NEXT:    CallBuiltin       r10, "HermesBuiltin.throwTypeError", 2
// CHECK-NEXT:    Unreachable

// CHECK:Function<?anon_0_?anon_0_simpleAsyncFE>(1 params, 24 registers):
// CHECK-NEXT:Offset in debug table: source 0x0037, lexical 0x0000
// CHECK-NEXT:    LoadParam         r0, 2
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 3
// CHECK-NEXT:    LoadConstUInt8    r4, 2
// CHECK-NEXT:    JStrictEqualLong  L1, r3, r4
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 3
// CHECK-NEXT:    LoadConstUInt8    r4, 3
// CHECK-NEXT:    JStrictEqualLong  L2, r3, r4
// CHECK-NEXT:    LoadConstUInt8    r3, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r3
// CHECK-NEXT:    LoadFromEnvironment r4, r2, 5
// CHECK-NEXT:    LoadConstZero     r3
// CHECK-NEXT:    StrictEq          r5, r3, r4
// CHECK-NEXT:    JmpTrue           L3, r5
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 2
// CHECK-NEXT:    Mov               r3, r5
// CHECK-NEXT:    LoadConstUInt8    r6, 1
// CHECK-NEXT:    JStrictEqual      L4, r1, r6
// CHECK-NEXT:    StoreToEnvironment r2, 4, r0
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r6, 2
// CHECK-NEXT:    StrictEq          r7, r1, r6
// CHECK-NEXT:    Mov               r3, r7
// CHECK-NEXT:    Mov               r8, r3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r8
// CHECK-NEXT:    LoadFromEnvironment r9, r2, 2
// CHECK-NEXT:    JmpTrue           L5, r9
// CHECK-NEXT:    LoadFromEnvironment r6, r2, 6
// CHECK-NEXT:    StoreToEnvironment r6, 0, r5
// CHECK-NEXT:    LoadFromEnvironment r7, r6, 0
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    NewObjectWithBuffer r9, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r9, r7, 0
// CHECK-NEXT:    Ret               r9
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r6
// CHECK-NEXT:    NewObjectWithBuffer r7, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r7, r5, 0
// CHECK-NEXT:    Ret               r7
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r6
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadFromEnvironment r7, r2, 1
// CHECK-NEXT:    Mov               r6, r7
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    JStrictEqual      L6, r1, r8
// CHECK-NEXT:    StoreToEnvironment r2, 4, r0
// CHECK-NEXT:    LoadFromEnvironment r7, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    StrictEq          r9, r1, r8
// CHECK-NEXT:    Mov               r6, r9
// CHECK-NEXT:    Mov               r10, r6
// CHECK-NEXT:    StoreNPToEnvironment r2, 1, r10
// CHECK-NEXT:    LoadFromEnvironment r11, r2, 1
// CHECK-NEXT:    JmpTrue           L7, r11
// CHECK-NEXT:    GetEnvironment    r8, r2, 1
// CHECK-NEXT:    CreateEnvironment r9, r8, 1
// CHECK-NEXT:    StoreToEnvironment r2, 6, r9
// CHECK-NEXT:    LoadConstUndefined r10
// CHECK-NEXT:    StoreNPToEnvironment r9, 0, r10
// CHECK-NEXT:    LoadConstUInt8    r11, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 5, r11
// CHECK-NEXT:    LoadConstUInt8    r12, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r12
// CHECK-NEXT:    NewObjectWithBuffer r13, 2, 2, 0, 8
// CHECK-NEXT:    Ret               r13
// CHECK-NEXT:L7:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    NewObjectWithBuffer r9, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r9, r7, 0
// CHECK-NEXT:    Ret               r9
// CHECK-NEXT:L6:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    JStrictEqual      L8, r1, r8
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    JStrictEqual      L9, r1, r8
// CHECK-NEXT:    NewObjectWithBuffer r8, 2, 2, 0, 6
// CHECK-NEXT:    LoadConstUndefined r9
// CHECK-NEXT:    PutOwnBySlotIdx   r8, r9, 0
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L9:
// CHECK-NEXT:    NewObjectWithBuffer r8, 2, 2, 0, 6
// CHECK-NEXT:    PutOwnBySlotIdx   r8, r0, 0
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L8:
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    LoadConstString   r9, "Generator functio"...
// CHECK-NEXT:    Mov               r15, r9
// CHECK-NEXT:    CallBuiltin       r10, "HermesBuiltin.throwTypeError", 2
// CHECK-NEXT:    Unreachable

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}async-function.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 6: line 10 col 1
// CHECK-NEXT:    bc 11: line 10 col 1
// CHECK-NEXT:    bc 16: line 10 col 1
// CHECK-NEXT:    bc 28: line 10 col 1
// CHECK-NEXT:    bc 41: line 10 col 1
// CHECK-NEXT:    bc 59: line 19 col 19
// CHECK-NEXT:  0x0016  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 34: line 10 col 1
// CHECK-NEXT:  0x001d  function idx 2, starts at line 14 col 1
// CHECK-NEXT:    bc 34: line 14 col 1
// CHECK-NEXT:  0x0024  function idx 3, starts at line 19 col 21
// CHECK-NEXT:    bc 34: line 19 col 21
// CHECK-NEXT:  0x002b  function idx 8, starts at line 14 col 1
// CHECK-NEXT:    bc 168: line 15 col 11
// CHECK-NEXT:    bc 343: line 15 col 11
// CHECK-NEXT:  0x0037  function idx 9, starts at line 19 col 21
// CHECK-NEXT:    bc 168: line 20 col 11
// CHECK-NEXT:    bc 343: line 20 col 11
// CHECK-NEXT:  0x0043  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
