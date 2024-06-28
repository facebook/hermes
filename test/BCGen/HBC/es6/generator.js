/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -enable-asserts -O0 -dump-bytecode %s | %FileCheckOrRegen %s --match-full-lines

function *loop(x) {
  var i = 0;
  while (y) {
    yield x[i++];
  }
  return 'DONE LOOPING';
}

function *args() {
  yield arguments[0];
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 5
// CHECK-NEXT:  String count: 11
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 2
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..-1]:
// CHECK-NEXT:s1[ASCII, 0..11]: DONE LOOPING
// CHECK-NEXT:s2[ASCII, 12..72]: Generator functions may not be called on executing generators
// CHECK-NEXT:s3[ASCII, 73..82]: args?inner
// CHECK-NEXT:s4[ASCII, 83..88]: global
// CHECK-NEXT:s5[ASCII, 89..98]: loop?inner
// CHECK-NEXT:i6[ASCII, 99..102] #50273FEB: args
// CHECK-NEXT:i7[ASCII, 103..106] #B553E7BD: done
// CHECK-NEXT:i8[ASCII, 107..110] #EFC200CF: loop
// CHECK-NEXT:i9[ASCII, 111..115] #DF50693D: value
// CHECK-NEXT:i10[ASCII, 116..116] #0001E3E8: y

// CHECK:Literal Value Buffer:
// CHECK-NEXT:null
// CHECK-NEXT:true
// CHECK-NEXT:null
// CHECK-NEXT:false
// CHECK-NEXT:[String 1]
// CHECK-NEXT:true
// CHECK-NEXT:Object Key Buffer:
// CHECK-NEXT:[String 9]
// CHECK-NEXT:[String 7]
// CHECK-NEXT:Object Shape Table:
// CHECK-NEXT:0[0, 2]
// CHECK-NEXT:Function Source Table:
// CHECK-NEXT:  Function ID 3 -> s0
// CHECK-NEXT:  Function ID 4 -> s0

// CHECK:Function<global>(1 params, 9 registers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r0, 0
// CHECK-NEXT:    DeclareGlobalVar  "loop"
// CHECK-NEXT:    DeclareGlobalVar  "args"
// CHECK-NEXT:    CreateClosure     r1, r0, NCFunction<loop>
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "loop"
// CHECK-NEXT:    CreateClosure     r3, r0, NCFunction<args>
// CHECK-NEXT:    GetGlobalObject   r4
// CHECK-NEXT:    PutByIdLoose      r4, r3, 2, "args"
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    Mov               r5, r6
// CHECK-NEXT:    Mov               r7, r5
// CHECK-NEXT:    Ret               r7

// CHECK:NCFunction<loop>(2 params, 8 registers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    CreateEnvironment r1, r0, 8
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadParam         r3, 1
// CHECK-NEXT:    StoreToEnvironment r1, 1, r3
// CHECK-NEXT:    LoadConstZero     r4
// CHECK-NEXT:    StoreNPToEnvironment r1, 6, r4
// CHECK-NEXT:    LoadConstZero     r5
// CHECK-NEXT:    StoreNPToEnvironment r1, 7, r5
// CHECK-NEXT:    CreateGenerator   r6, r1, Function<loop?inner>
// CHECK-NEXT:    Ret               r6

// CHECK:NCFunction<args>(1 params, 10 registers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    CreateEnvironment r1, r0, 7
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    Mov               r3, r4
// CHECK-NEXT:    ReifyArgumentsLoose r3
// CHECK-NEXT:    Mov               r5, r3
// CHECK-NEXT:    StoreToEnvironment r1, 1, r5
// CHECK-NEXT:    LoadConstZero     r6
// CHECK-NEXT:    StoreNPToEnvironment r1, 6, r6
// CHECK-NEXT:    LoadConstZero     r7
// CHECK-NEXT:    StoreNPToEnvironment r1, 4, r7
// CHECK-NEXT:    CreateGenerator   r8, r1, Function<args?inner>
// CHECK-NEXT:    Ret               r8

// CHECK:Function<loop?inner>(2 params, 27 registers):
// CHECK-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHECK-NEXT:    LoadParam         r0, 2
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 7
// CHECK-NEXT:    LoadConstUInt8    r4, 2
// CHECK-NEXT:    JStrictEqualLong  L1, r3, r4
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 7
// CHECK-NEXT:    LoadConstUInt8    r4, 3
// CHECK-NEXT:    JStrictEqualLong  L2, r3, r4
// CHECK-NEXT:    LoadConstUInt8    r3, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 7, r3
// CHECK-NEXT:    LoadFromEnvironment r4, r2, 6
// CHECK-NEXT:    LoadConstZero     r3
// CHECK-NEXT:    StrictEq          r5, r3, r4
// CHECK-NEXT:    JmpTrue           L3, r5
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 3
// CHECK-NEXT:    Mov               r3, r5
// CHECK-NEXT:    LoadConstUInt8    r6, 1
// CHECK-NEXT:    JStrictEqual      L4, r1, r6
// CHECK-NEXT:    StoreToEnvironment r2, 5, r0
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 5
// CHECK-NEXT:    LoadConstUInt8    r6, 2
// CHECK-NEXT:    StrictEq          r7, r1, r6
// CHECK-NEXT:    Mov               r3, r7
// CHECK-NEXT:    Mov               r8, r3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    LoadFromEnvironment r9, r2, 3
// CHECK-NEXT:    JmpTrue           L5, r9
// CHECK-NEXT:    GetGlobalObject   r6
// CHECK-NEXT:    TryGetById        r7, r6, 1, "y"
// CHECK-NEXT:    JmpTrueLong       L6, r7
// CHECK-NEXT:    JmpLong           L7
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 7, r6
// CHECK-NEXT:    NewObjectWithBuffer r7, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r7, r5, 0
// CHECK-NEXT:    Ret               r7
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 7, r6
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadFromEnvironment r7, r2, 2
// CHECK-NEXT:    Mov               r6, r7
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    JStrictEqualLong  L8, r1, r8
// CHECK-NEXT:    StoreToEnvironment r2, 5, r0
// CHECK-NEXT:    LoadFromEnvironment r7, r2, 5
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    StrictEq          r9, r1, r8
// CHECK-NEXT:    Mov               r6, r9
// CHECK-NEXT:    Mov               r10, r6
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r10
// CHECK-NEXT:    LoadFromEnvironment r11, r2, 2
// CHECK-NEXT:    JmpTrue           L9, r11
// CHECK-NEXT:    GetEnvironment    r8, r2, 1
// CHECK-NEXT:    CreateEnvironment r9, r8, 2
// CHECK-NEXT:    StoreToEnvironment r2, 4, r9
// CHECK-NEXT:    LoadFromEnvironment r10, r2, 1
// CHECK-NEXT:    StoreToEnvironment r9, 0, r10
// CHECK-NEXT:    LoadConstUndefined r11
// CHECK-NEXT:    StoreNPToEnvironment r9, 1, r11
// CHECK-NEXT:    LoadConstZero     r12
// CHECK-NEXT:    StoreNPToEnvironment r9, 1, r12
// CHECK-NEXT:    GetGlobalObject   r13
// CHECK-NEXT:    TryGetById        r14, r13, 1, "y"
// CHECK-NEXT:    JmpTrue           L6, r14
// CHECK-NEXT:L7:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 7, r8
// CHECK-NEXT:    NewObjectWithBuffer r9, 0, 4
// CHECK-NEXT:    Ret               r9
// CHECK-NEXT:L6:
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 4
// CHECK-NEXT:    LoadFromEnvironment r9, r8, 0
// CHECK-NEXT:    LoadFromEnvironment r10, r8, 1
// CHECK-NEXT:    ToNumeric         r11, r10
// CHECK-NEXT:    Inc               r12, r11
// CHECK-NEXT:    StoreToEnvironment r8, 1, r12
// CHECK-NEXT:    GetByVal          r13, r9, r11
// CHECK-NEXT:    LoadConstUInt8    r14, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 6, r14
// CHECK-NEXT:    LoadConstUInt8    r15, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 7, r15
// CHECK-NEXT:    NewObjectWithBuffer r16, 0, 2
// CHECK-NEXT:    PutOwnBySlotIdx   r16, r13, 0
// CHECK-NEXT:    Ret               r16
// CHECK-NEXT:L9:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 7, r8
// CHECK-NEXT:    NewObjectWithBuffer r9, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r9, r7, 0
// CHECK-NEXT:    Ret               r9
// CHECK-NEXT:L8:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 7, r8
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    JStrictEqual      L10, r1, r8
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    JStrictEqual      L11, r1, r8
// CHECK-NEXT:    NewObjectWithBuffer r8, 0, 0
// CHECK-NEXT:    LoadConstUndefined r9
// CHECK-NEXT:    PutOwnBySlotIdx   r8, r9, 0
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L11:
// CHECK-NEXT:    NewObjectWithBuffer r8, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r8, r0, 0
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L10:
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 7, r8
// CHECK-NEXT:    LoadConstString   r9, "Generator functio"...
// CHECK-NEXT:    Mov               r18, r9
// CHECK-NEXT:    CallBuiltin       r10, "HermesBuiltin.throwTypeError", 2
// CHECK-NEXT:    Unreachable

// CHECK:Function<args?inner>(1 params, 24 registers):
// CHECK-NEXT:Offset in debug table: source 0x0029, lexical 0x0000
// CHECK-NEXT:    LoadParam         r0, 2
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r4, 2
// CHECK-NEXT:    JStrictEqualLong  L1, r3, r4
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r4, 3
// CHECK-NEXT:    JStrictEqualLong  L2, r3, r4
// CHECK-NEXT:    LoadConstUInt8    r3, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r3
// CHECK-NEXT:    LoadFromEnvironment r4, r2, 6
// CHECK-NEXT:    LoadConstZero     r3
// CHECK-NEXT:    StrictEq          r5, r3, r4
// CHECK-NEXT:    JmpTrue           L3, r5
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 3
// CHECK-NEXT:    Mov               r3, r5
// CHECK-NEXT:    LoadConstUInt8    r6, 1
// CHECK-NEXT:    JStrictEqual      L4, r1, r6
// CHECK-NEXT:    StoreToEnvironment r2, 5, r0
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 5
// CHECK-NEXT:    LoadConstUInt8    r6, 2
// CHECK-NEXT:    StrictEq          r7, r1, r6
// CHECK-NEXT:    Mov               r3, r7
// CHECK-NEXT:    Mov               r8, r3
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    LoadFromEnvironment r9, r2, 3
// CHECK-NEXT:    JmpTrue           L5, r9
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r6
// CHECK-NEXT:    NewObjectWithBuffer r7, 0, 0
// CHECK-NEXT:    LoadConstUndefined r8
// CHECK-NEXT:    PutOwnBySlotIdx   r7, r8, 0
// CHECK-NEXT:    Ret               r7
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r6
// CHECK-NEXT:    NewObjectWithBuffer r7, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r7, r5, 0
// CHECK-NEXT:    Ret               r7
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r6
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadFromEnvironment r7, r2, 2
// CHECK-NEXT:    Mov               r6, r7
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    JStrictEqual      L6, r1, r8
// CHECK-NEXT:    StoreToEnvironment r2, 5, r0
// CHECK-NEXT:    LoadFromEnvironment r7, r2, 5
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    StrictEq          r9, r1, r8
// CHECK-NEXT:    Mov               r6, r9
// CHECK-NEXT:    Mov               r10, r6
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r10
// CHECK-NEXT:    LoadFromEnvironment r11, r2, 2
// CHECK-NEXT:    JmpTrue           L7, r11
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 1
// CHECK-NEXT:    GetEnvironment    r9, r2, 1
// CHECK-NEXT:    CreateEnvironment r10, r9, 0
// CHECK-NEXT:    GetByIndex        r10, r8, 0
// CHECK-NEXT:    LoadConstUInt8    r11, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 6, r11
// CHECK-NEXT:    LoadConstUInt8    r12, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r12
// CHECK-NEXT:    NewObjectWithBuffer r13, 0, 2
// CHECK-NEXT:    PutOwnBySlotIdx   r13, r10, 0
// CHECK-NEXT:    Ret               r13
// CHECK-NEXT:L7:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r8
// CHECK-NEXT:    NewObjectWithBuffer r9, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r9, r7, 0
// CHECK-NEXT:    Ret               r9
// CHECK-NEXT:L6:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r8
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    JStrictEqual      L8, r1, r8
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    JStrictEqual      L9, r1, r8
// CHECK-NEXT:    NewObjectWithBuffer r8, 0, 0
// CHECK-NEXT:    LoadConstUndefined r9
// CHECK-NEXT:    PutOwnBySlotIdx   r8, r9, 0
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L9:
// CHECK-NEXT:    NewObjectWithBuffer r8, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r8, r0, 0
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L8:
// CHECK-NEXT:    Throw             r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r8
// CHECK-NEXT:    LoadConstString   r9, "Generator functio"...
// CHECK-NEXT:    Mov               r15, r9
// CHECK-NEXT:    CallBuiltin       r10, "HermesBuiltin.throwTypeError", 2
// CHECK-NEXT:    Unreachable

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}generator.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 6: line 10 col 1
// CHECK-NEXT:    bc 11: line 10 col 1
// CHECK-NEXT:    bc 23: line 10 col 1
// CHECK-NEXT:    bc 36: line 10 col 1
// CHECK-NEXT:  0x0010  function idx 3, starts at line 10 col 1
// CHECK-NEXT:    bc 105: line 12 col 10
// CHECK-NEXT:    bc 148: line 13 col 5
// CHECK-NEXT:    bc 236: line 12 col 10
// CHECK-NEXT:    bc 272: line 13 col 14
// CHECK-NEXT:    bc 282: line 13 col 12
// CHECK-NEXT:    bc 380: line 13 col 5
// CHECK-NEXT:  0x0029  function idx 4, starts at line 18 col 1
// CHECK-NEXT:    bc 150: line 19 col 3
// CHECK-NEXT:    bc 213: line 19 col 18
// CHECK-NEXT:    bc 311: line 19 col 3
// CHECK-NEXT:  0x0038  end of debug source table
