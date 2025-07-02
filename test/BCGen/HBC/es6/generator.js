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
// CHECK-NEXT:  String count: 9
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  StringSwitchImm count: 0
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
// CHECK-NEXT:s3[ASCII, 73..78]: global
// CHECK-NEXT:i4[ASCII, 79..82] #50273FEB: args
// CHECK-NEXT:i5[ASCII, 83..86] #B553E7BD: done
// CHECK-NEXT:i6[ASCII, 87..90] #EFC200CF: loop
// CHECK-NEXT:i7[ASCII, 91..95] #DF50693D: value
// CHECK-NEXT:i8[ASCII, 96..96] #0001E3E8: y

// CHECK:Literal Value Buffer:
// CHECK-NEXT:null
// CHECK-NEXT:true
// CHECK-NEXT:undefined
// CHECK-NEXT:true
// CHECK-NEXT:null
// CHECK-NEXT:false
// CHECK-NEXT:[String 1]
// CHECK-NEXT:true
// CHECK-NEXT:Object Key Buffer:
// CHECK-NEXT:[String 7]
// CHECK-NEXT:[String 5]
// CHECK-NEXT:Object Shape Table:
// CHECK-NEXT:0[0, 2]
// CHECK-NEXT:Function Source Table:
// CHECK-NEXT:  Function ID 3 -> s0
// CHECK-NEXT:  Function ID 4 -> s0

// CHECK:Function<global>(1 params, 4 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r1, 0
// CHECK-NEXT:    DeclareGlobalVar  "loop"
// CHECK-NEXT:    DeclareGlobalVar  "args"
// CHECK-NEXT:    CreateClosure     r2, r1, NCFunction<loop>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdLoose      r3, r2, 0, "loop"
// CHECK-NEXT:    CreateClosure     r1, r1, NCFunction<args>
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "args"
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r1, r2
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<loop>(2 params, 3 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r1, 0
// CHECK-NEXT:    CreateEnvironment r1, r1, 10
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadParam         r2, 1
// CHECK-NEXT:    StoreToEnvironment r1, 1, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 6, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 9, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 8, r2
// CHECK-NEXT:    CreateGenerator   r1, r1, Function<loop>
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<args>(1 params, 4 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r1, 0
// CHECK-NEXT:    CreateEnvironment r1, r1, 9
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Mov               r2, r3
// CHECK-NEXT:    ReifyArgumentsLoose r2
// CHECK-NEXT:    StoreToEnvironment r1, 1, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 6, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 4, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 8, r2
// CHECK-NEXT:    CreateGenerator   r1, r1, Function<args>
// CHECK-NEXT:    Ret               r1

// CHECK:Function<loop>(2 params, 23 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0011, lexical 0x0000
// CHECK-NEXT:    LoadParam         r3, 2
// CHECK-NEXT:    LoadParam         r4, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 9
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r8, r8, r1
// CHECK-NEXT:    JmpTrueLong       L1, r8
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 9
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StrictEq          r8, r8, r1
// CHECK-NEXT:    JmpTrueLong       L2, r8
// CHECK-NEXT:L13:
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 9, r8
// CHECK-NEXT:    LoadFromEnvironment r13, r2, 6
// CHECK-NEXT:    LoadConstZero     r8
// CHECK-NEXT:    StrictEq          r8, r8, r13
// CHECK-NEXT:    JmpTrue           L3, r8
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 3
// CHECK-NEXT:    Mov               r12, r8
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    StrictEq          r8, r4, r8
// CHECK-NEXT:    JmpTrue           L4, r8
// CHECK-NEXT:    StoreToEnvironment r2, 5, r3
// CHECK-NEXT:    LoadFromEnvironment r11, r2, 5
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    StrictEq          r8, r4, r8
// CHECK-NEXT:    Mov               r12, r8
// CHECK-NEXT:    Mov               r8, r12
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r8
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 3
// CHECK-NEXT:    JmpTrue           L5, r8
// CHECK-NEXT:    GetGlobalObject   r8
// CHECK-NEXT:    TryGetById        r8, r8, 0, "y"
// CHECK-NEXT:    JmpTrueLong       L6, r8
// CHECK-NEXT:    JmpLong           L7
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 9, r8
// CHECK-NEXT:L14:
// CHECK-NEXT:    NewObjectWithBuffer r8, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r8, r11, 0
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 9, r8
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 2
// CHECK-NEXT:    Mov               r10, r8
// CHECK-NEXT:    LoadConstUInt8    r8, 1
// CHECK-NEXT:    StrictEq          r8, r4, r8
// CHECK-NEXT:    JmpTrueLong       L8, r8
// CHECK-NEXT:    StoreToEnvironment r2, 5, r3
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 5
// CHECK-NEXT:    LoadConstUInt8    r8, 2
// CHECK-NEXT:    StrictEq          r8, r4, r8
// CHECK-NEXT:    Mov               r10, r8
// CHECK-NEXT:    Mov               r8, r10
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r8
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 2
// CHECK-NEXT:    JmpTrue           L9, r8
// CHECK-NEXT:    GetEnvironment    r8, r2, 1
// CHECK-NEXT:    CreateEnvironment r8, r8, 2
// CHECK-NEXT:    StoreToEnvironment r2, 4, r8
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 1
// CHECK-NEXT:    StoreToEnvironment r8, 0, r1
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r8, 1, r1
// CHECK-NEXT:    LoadConstZero     r1
// CHECK-NEXT:    StoreNPToEnvironment r8, 1, r1
// CHECK-NEXT:    GetGlobalObject   r8
// CHECK-NEXT:    TryGetById        r8, r8, 0, "y"
// CHECK-NEXT:    JmpTrue           L6, r8
// CHECK-NEXT:L7:
// CHECK-NEXT:    LoadConstUInt8    r8, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 9, r8
// CHECK-NEXT:L15:
// CHECK-NEXT:    NewObjectWithBuffer r8, 0, 6
// CHECK-NEXT:    Ret               r8
// CHECK-NEXT:L6:
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 4
// CHECK-NEXT:    LoadFromEnvironment r1, r8, 0
// CHECK-NEXT:    LoadFromEnvironment r7, r8, 1
// CHECK-NEXT:    ToNumeric         r7, r7
// CHECK-NEXT:    Inc               r9, r7
// CHECK-NEXT:    StoreToEnvironment r8, 1, r9
// CHECK-NEXT:    GetByVal          r6, r1, r7
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 6, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 9, r1
// CHECK-NEXT:L16:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 4
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r6, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L9:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 9, r1
// CHECK-NEXT:L17:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r5, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L8:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 9, r1
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L10:
// CHECK-NEXT:    Catch             r1
// CHECK-NEXT:    StoreToEnvironment r2, 7, r1
// CHECK-NEXT:    LoadFromEnvironment r0, r2, 8
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 9, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 7
// CHECK-NEXT:    Throw             r1
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L11, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L12, r1
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 2
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L12:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r3, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L11:
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 9, r1
// CHECK-NEXT:    LoadConstString   r1, "Generator functio"...
// CHECK-NEXT:    Mov               r14, r1
// CHECK-NEXT:    CallBuiltin       r0, "HermesBuiltin.throwTypeError", 2
// CHECK-NEXT:    Unreachable

// CHECK:Exception Handlers:
// CHECK-NEXT:0: start = L13, end = L14, target = L10
// CHECK-NEXT:1: start = L4, end = L15, target = L10
// CHECK-NEXT:2: start = L6, end = L16, target = L10
// CHECK-NEXT:3: start = L9, end = L17, target = L10
// CHECK-NEXT:4: start = L8, end = L10, target = L10

// CHECK:Function<args>(1 params, 21 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x003c, lexical 0x0000
// CHECK-NEXT:    LoadParam         r3, 2
// CHECK-NEXT:    LoadParam         r4, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r7, 2
// CHECK-NEXT:    StrictEq          r1, r1, r7
// CHECK-NEXT:    JmpTrueLong       L1, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r7, 3
// CHECK-NEXT:    StrictEq          r1, r1, r7
// CHECK-NEXT:    JmpTrueLong       L2, r1
// CHECK-NEXT:L11:
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r1
// CHECK-NEXT:    LoadFromEnvironment r11, r2, 6
// CHECK-NEXT:    LoadConstZero     r1
// CHECK-NEXT:    StrictEq          r1, r1, r11
// CHECK-NEXT:    JmpTrue           L3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 3
// CHECK-NEXT:    Mov               r10, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L4, r1
// CHECK-NEXT:    StoreToEnvironment r2, 5, r3
// CHECK-NEXT:    LoadFromEnvironment r9, r2, 5
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    Mov               r10, r1
// CHECK-NEXT:    Mov               r1, r10
// CHECK-NEXT:    StoreNPToEnvironment r2, 3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 3
// CHECK-NEXT:    JmpTrue           L5, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r1
// CHECK-NEXT:L12:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 2
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r1
// CHECK-NEXT:L13:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r9, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r1
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 2
// CHECK-NEXT:    Mov               r8, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L6, r1
// CHECK-NEXT:    StoreToEnvironment r2, 5, r3
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 5
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    Mov               r8, r1
// CHECK-NEXT:    Mov               r1, r8
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 2
// CHECK-NEXT:    JmpTrue           L7, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 1
// CHECK-NEXT:    GetEnvironment    r7, r2, 1
// CHECK-NEXT:    CreateEnvironment r0, r7, 0
// CHECK-NEXT:    GetByIndex        r6, r1, 0
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 6, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r1
// CHECK-NEXT:L14:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 4
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r6, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L7:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r1
// CHECK-NEXT:L15:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r5, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L6:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r1
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L8:
// CHECK-NEXT:    Catch             r1
// CHECK-NEXT:    StoreToEnvironment r2, 7, r1
// CHECK-NEXT:    LoadFromEnvironment r0, r2, 8
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 7
// CHECK-NEXT:    Throw             r1
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L9, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L10, r1
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 2
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L10:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r3, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L9:
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 4, r1
// CHECK-NEXT:    LoadConstString   r1, "Generator functio"...
// CHECK-NEXT:    Mov               r12, r1
// CHECK-NEXT:    CallBuiltin       r0, "HermesBuiltin.throwTypeError", 2
// CHECK-NEXT:    Unreachable

// CHECK:Exception Handlers:
// CHECK-NEXT:0: start = L11, end = L12, target = L8
// CHECK-NEXT:1: start = L5, end = L13, target = L8
// CHECK-NEXT:2: start = L4, end = L14, target = L8
// CHECK-NEXT:3: start = L7, end = L15, target = L8
// CHECK-NEXT:4: start = L6, end = L8, target = L8

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
// CHECK-NEXT:  0x0011  function idx 3, starts at line 10 col 1
// CHECK-NEXT:    bc 43: line 13 col 5
// CHECK-NEXT:    bc 114: line 12 col 10
// CHECK-NEXT:    bc 138: line 13 col 5
// CHECK-NEXT:    bc 157: line 13 col 5
// CHECK-NEXT:    bc 248: line 12 col 10
// CHECK-NEXT:    bc 264: line 15 col 3
// CHECK-NEXT:    bc 284: line 13 col 14
// CHECK-NEXT:    bc 294: line 13 col 12
// CHECK-NEXT:    bc 312: line 13 col 5
// CHECK-NEXT:    bc 352: line 13 col 5
// CHECK-NEXT:    bc 373: line 13 col 5
// CHECK-NEXT:    bc 415: line 13 col 5
// CHECK-NEXT:  0x003c  function idx 4, starts at line 18 col 1
// CHECK-NEXT:    bc 43: line 19 col 3
// CHECK-NEXT:    bc 119: line 20 col 1
// CHECK-NEXT:    bc 134: line 19 col 3
// CHECK-NEXT:    bc 153: line 19 col 3
// CHECK-NEXT:    bc 219: line 19 col 18
// CHECK-NEXT:    bc 237: line 19 col 3
// CHECK-NEXT:    bc 277: line 19 col 3
// CHECK-NEXT:    bc 298: line 19 col 3
// CHECK-NEXT:    bc 340: line 19 col 3
// CHECK-NEXT:  0x005e  end of debug source table
