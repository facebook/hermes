/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -enable-asserts -dump-bytecode %s | %FileCheckOrRegen %s --match-full-lines

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
// CHECK-NEXT:  String count: 11
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  StringSwitchImm count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 3
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..-1]:
// CHECK-NEXT:s1[ASCII, 0..20]: ?anon_0_simpleAsyncFE
// CHECK-NEXT:s2[ASCII, 21..39]: ?anon_0_simpleAwait
// CHECK-NEXT:s3[ASCII, 40..59]: ?anon_0_simpleReturn
// CHECK-NEXT:s4[ASCII, 60..120]: Generator functions may not be called on executing generators
// CHECK-NEXT:s5[ASCII, 121..126]: global
// CHECK-NEXT:i6[ASCII, 127..130] #B553E7BD: done
// CHECK-NEXT:i7[ASCII, 131..143] #4CCB9499: simpleAsyncFE
// CHECK-NEXT:i8[ASCII, 144..154] #FD482E4F: simpleAwait
// CHECK-NEXT:i9[ASCII, 155..166] #EB416734: simpleReturn
// CHECK-NEXT:i10[ASCII, 167..171] #DF50693D: value

// CHECK:Literal Value Buffer:
// CHECK-NEXT:null
// CHECK-NEXT:true
// CHECK-NEXT:undefined
// CHECK-NEXT:true
// CHECK-NEXT:[int 1]
// CHECK-NEXT:true
// CHECK-NEXT:[int 2]
// CHECK-NEXT:false
// CHECK-NEXT:Object Key Buffer:
// CHECK-NEXT:[String 10]
// CHECK-NEXT:[String 6]
// CHECK-NEXT:Object Shape Table:
// CHECK-NEXT:0[0, 2]
// CHECK-NEXT:Function Source Table:
// CHECK-NEXT:  Function ID 7 -> s0
// CHECK-NEXT:  Function ID 8 -> s0
// CHECK-NEXT:  Function ID 9 -> s0

// CHECK:Function<global>(1 params, 4 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r2, 0
// CHECK-NEXT:    DeclareGlobalVar  "simpleReturn"
// CHECK-NEXT:    DeclareGlobalVar  "simpleAwait"
// CHECK-NEXT:    DeclareGlobalVar  "simpleAsyncFE"
// CHECK-NEXT:    CreateClosure     r1, r2, NCFunction<simpleReturn>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdLoose      r3, r1, 0, "simpleReturn"
// CHECK-NEXT:    CreateClosure     r1, r2, NCFunction<simpleAwait>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdLoose      r3, r1, 1, "simpleAwait"
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Mov               r1, r3
// CHECK-NEXT:    CreateClosure     r2, r2, NCFunction<simpleAsyncFE>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdLoose      r3, r2, 2, "simpleAsyncFE"
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<simpleReturn>(1 params, 17 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0018
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    Mov               r5, r4
// CHECK-NEXT:    LoadParam         r4, 0
// CHECK-NEXT:    CoerceThisNS      r4, r4
// CHECK-NEXT:    GetParentEnvironment r3, 0
// CHECK-NEXT:    CreateEnvironment r3, r3, 0
// CHECK-NEXT:    CreateClosure     r3, r3, NCFunction<?anon_0_simpleReturn>
// CHECK-NEXT:    GetBuiltinClosure r1, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArgumentsLoose r5
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Call4             r1, r1, r2, r3, r4, r5
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<simpleAwait>(1 params, 17 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0020
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    Mov               r5, r4
// CHECK-NEXT:    LoadParam         r4, 0
// CHECK-NEXT:    CoerceThisNS      r4, r4
// CHECK-NEXT:    GetParentEnvironment r3, 0
// CHECK-NEXT:    CreateEnvironment r3, r3, 0
// CHECK-NEXT:    CreateClosure     r3, r3, NCFunction<?anon_0_simpleAwait>
// CHECK-NEXT:    GetBuiltinClosure r1, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArgumentsLoose r5
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Call4             r1, r1, r2, r3, r4, r5
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<simpleAsyncFE>(1 params, 17 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0028
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    Mov               r5, r4
// CHECK-NEXT:    LoadParam         r4, 0
// CHECK-NEXT:    CoerceThisNS      r4, r4
// CHECK-NEXT:    GetParentEnvironment r3, 0
// CHECK-NEXT:    CreateEnvironment r3, r3, 0
// CHECK-NEXT:    CreateClosure     r3, r3, NCFunction<?anon_0_simpleAsyncFE>
// CHECK-NEXT:    GetBuiltinClosure r1, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArgumentsLoose r5
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Call4             r1, r1, r2, r3, r4, r5
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<?anon_0_simpleReturn>(1 params, 3 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r1, 0
// CHECK-NEXT:    CreateEnvironment r1, r1, 7
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 4, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 2, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 6, r2
// CHECK-NEXT:    CreateGenerator   r1, r1, Function<?anon_0_simpleReturn>
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<?anon_0_simpleAwait>(1 params, 3 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r1, 0
// CHECK-NEXT:    CreateEnvironment r1, r1, 9
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 5, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 8, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 7, r2
// CHECK-NEXT:    CreateGenerator   r1, r1, Function<?anon_0_simpleAwait>
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<?anon_0_simpleAsyncFE>(1 params, 3 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    GetParentEnvironment r1, 0
// CHECK-NEXT:    CreateEnvironment r1, r1, 9
// CHECK-NEXT:    LoadParam         r2, 0
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 5, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 8, r2
// CHECK-NEXT:    LoadConstZero     r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 7, r2
// CHECK-NEXT:    CreateGenerator   r1, r1, Function<?anon_0_simpleAsyncFE>
// CHECK-NEXT:    Ret               r1

// CHECK:Function<?anon_0_simpleReturn>(1 params, 17 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0030
// CHECK-NEXT:    LoadParam         r3, 2
// CHECK-NEXT:    LoadParam         r4, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 2
// CHECK-NEXT:    LoadConstUInt8    r7, 2
// CHECK-NEXT:    StrictEq          r1, r1, r7
// CHECK-NEXT:    JmpTrueLong       L1, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 2
// CHECK-NEXT:    LoadConstUInt8    r7, 3
// CHECK-NEXT:    StrictEq          r1, r1, r7
// CHECK-NEXT:    JmpTrueLong       L2, r1
// CHECK-NEXT:L8:
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r1
// CHECK-NEXT:    LoadFromEnvironment r0, r2, 4
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 1
// CHECK-NEXT:    Mov               r6, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L3, r1
// CHECK-NEXT:    StoreToEnvironment r2, 3, r3
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 3
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    Mov               r6, r1
// CHECK-NEXT:    Mov               r1, r6
// CHECK-NEXT:    StoreNPToEnvironment r2, 1, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 1
// CHECK-NEXT:    JmpTrue           L4, r1
// CHECK-NEXT:    GetEnvironment    r1, r2, 1
// CHECK-NEXT:    CreateEnvironment r0, r1, 0
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r1
// CHECK-NEXT:L9:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 4
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r1
// CHECK-NEXT:L10:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r5, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r1
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L5:
// CHECK-NEXT:    Catch             r1
// CHECK-NEXT:    StoreToEnvironment r2, 5, r1
// CHECK-NEXT:    LoadFromEnvironment r0, r2, 6
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 5
// CHECK-NEXT:    Throw             r1
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L6, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L7, r1
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 2
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L7:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r3, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L6:
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L1:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r1
// CHECK-NEXT:    LoadConstString   r1, "Generator functio"...
// CHECK-NEXT:    Mov               r8, r1
// CHECK-NEXT:    CallBuiltin       r0, "HermesBuiltin.throwTypeError", 2
// CHECK-NEXT:    Unreachable

// CHECK:Exception Handlers:
// CHECK-NEXT:0: start = L8, end = L9, target = L5
// CHECK-NEXT:1: start = L4, end = L10, target = L5
// CHECK-NEXT:2: start = L3, end = L5, target = L5

// CHECK:Function<?anon_0_simpleAwait>(1 params, 21 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x003d
// CHECK-NEXT:    LoadParam         r3, 2
// CHECK-NEXT:    LoadParam         r4, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 8
// CHECK-NEXT:    LoadConstUInt8    r6, 2
// CHECK-NEXT:    StrictEq          r1, r1, r6
// CHECK-NEXT:    JmpTrueLong       L1, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 8
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StrictEq          r1, r1, r6
// CHECK-NEXT:    JmpTrueLong       L2, r1
// CHECK-NEXT:L11:
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:    LoadFromEnvironment r11, r2, 5
// CHECK-NEXT:    LoadConstZero     r1
// CHECK-NEXT:    StrictEq          r1, r1, r11
// CHECK-NEXT:    JmpTrue           L3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 2
// CHECK-NEXT:    Mov               r10, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L4, r1
// CHECK-NEXT:    StoreToEnvironment r2, 4, r3
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    Mov               r10, r1
// CHECK-NEXT:    Mov               r1, r10
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 2
// CHECK-NEXT:    JmpTrue           L5, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 3
// CHECK-NEXT:    StoreToEnvironment r1, 0, r8
// CHECK-NEXT:    LoadFromEnvironment r9, r1, 0
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:L12:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r9, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:L13:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r8, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 1
// CHECK-NEXT:    Mov               r7, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L6, r1
// CHECK-NEXT:    StoreToEnvironment r2, 4, r3
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    Mov               r7, r1
// CHECK-NEXT:    Mov               r1, r7
// CHECK-NEXT:    StoreNPToEnvironment r2, 1, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 1
// CHECK-NEXT:    JmpTrue           L7, r1
// CHECK-NEXT:    GetEnvironment    r1, r2, 1
// CHECK-NEXT:    CreateEnvironment r1, r1, 1
// CHECK-NEXT:    StoreToEnvironment r2, 3, r1
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    StoreNPToEnvironment r1, 0, r6
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 5, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:L14:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 10
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L7:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:L15:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r5, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L6:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L8:
// CHECK-NEXT:    Catch             r1
// CHECK-NEXT:    StoreToEnvironment r2, 6, r1
// CHECK-NEXT:    LoadFromEnvironment r0, r2, 7
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 6
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
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
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

// CHECK:Function<?anon_0_simpleAsyncFE>(1 params, 21 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x005c
// CHECK-NEXT:    LoadParam         r3, 2
// CHECK-NEXT:    LoadParam         r4, 1
// CHECK-NEXT:    GetParentEnvironment r2, 0
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 8
// CHECK-NEXT:    LoadConstUInt8    r6, 2
// CHECK-NEXT:    StrictEq          r1, r1, r6
// CHECK-NEXT:    JmpTrueLong       L1, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 8
// CHECK-NEXT:    LoadConstUInt8    r6, 3
// CHECK-NEXT:    StrictEq          r1, r1, r6
// CHECK-NEXT:    JmpTrueLong       L2, r1
// CHECK-NEXT:L11:
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:    LoadFromEnvironment r11, r2, 5
// CHECK-NEXT:    LoadConstZero     r1
// CHECK-NEXT:    StrictEq          r1, r1, r11
// CHECK-NEXT:    JmpTrue           L3, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 2
// CHECK-NEXT:    Mov               r10, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L4, r1
// CHECK-NEXT:    StoreToEnvironment r2, 4, r3
// CHECK-NEXT:    LoadFromEnvironment r8, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    Mov               r10, r1
// CHECK-NEXT:    Mov               r1, r10
// CHECK-NEXT:    StoreNPToEnvironment r2, 2, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 2
// CHECK-NEXT:    JmpTrue           L5, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 3
// CHECK-NEXT:    StoreToEnvironment r1, 0, r8
// CHECK-NEXT:    LoadFromEnvironment r9, r1, 0
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:L12:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r9, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:L13:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r8, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L4:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L3:
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 1
// CHECK-NEXT:    Mov               r7, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    JmpTrue           L6, r1
// CHECK-NEXT:    StoreToEnvironment r2, 4, r3
// CHECK-NEXT:    LoadFromEnvironment r5, r2, 4
// CHECK-NEXT:    LoadConstUInt8    r1, 2
// CHECK-NEXT:    StrictEq          r1, r4, r1
// CHECK-NEXT:    Mov               r7, r1
// CHECK-NEXT:    Mov               r1, r7
// CHECK-NEXT:    StoreNPToEnvironment r2, 1, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 1
// CHECK-NEXT:    JmpTrue           L7, r1
// CHECK-NEXT:    GetEnvironment    r1, r2, 1
// CHECK-NEXT:    CreateEnvironment r1, r1, 1
// CHECK-NEXT:    StoreToEnvironment r2, 3, r1
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    StoreNPToEnvironment r1, 0, r6
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 5, r1
// CHECK-NEXT:    LoadConstUInt8    r1, 1
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:L14:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 10
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L7:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:L15:
// CHECK-NEXT:    NewObjectWithBuffer r1, 0, 0
// CHECK-NEXT:    PutOwnBySlotIdx   r1, r5, 0
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L6:
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:    Throw             r3
// CHECK-NEXT:L8:
// CHECK-NEXT:    Catch             r1
// CHECK-NEXT:    StoreToEnvironment r2, 6, r1
// CHECK-NEXT:    LoadFromEnvironment r0, r2, 7
// CHECK-NEXT:    LoadConstUInt8    r1, 3
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
// CHECK-NEXT:    LoadFromEnvironment r1, r2, 6
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
// CHECK-NEXT:    StoreNPToEnvironment r2, 8, r1
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
// CHECK-NEXT:  0x0018  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 33: line 10 col 1
// CHECK-NEXT:  0x0020  function idx 2, starts at line 14 col 1
// CHECK-NEXT:    bc 33: line 14 col 1
// CHECK-NEXT:  0x0028  function idx 3, starts at line 19 col 21
// CHECK-NEXT:    bc 33: line 19 col 21
// CHECK-NEXT:  0x0030  function idx 7, starts at line 10 col 1
// CHECK-NEXT:    bc 121: line 11 col 3
// CHECK-NEXT:  0x003d  function idx 8, starts at line 14 col 1
// CHECK-NEXT:    bc 43: line 15 col 11
// CHECK-NEXT:    bc 131: line 16 col 3
// CHECK-NEXT:    bc 150: line 15 col 11
// CHECK-NEXT:    bc 169: line 15 col 11
// CHECK-NEXT:    bc 255: line 15 col 11
// CHECK-NEXT:    bc 291: line 15 col 11
// CHECK-NEXT:    bc 312: line 15 col 11
// CHECK-NEXT:    bc 354: line 15 col 11
// CHECK-NEXT:  0x005c  function idx 9, starts at line 19 col 21
// CHECK-NEXT:    bc 43: line 20 col 11
// CHECK-NEXT:    bc 131: line 21 col 3
// CHECK-NEXT:    bc 150: line 20 col 11
// CHECK-NEXT:    bc 169: line 20 col 11
// CHECK-NEXT:    bc 255: line 20 col 11
// CHECK-NEXT:    bc 291: line 20 col 11
// CHECK-NEXT:    bc 312: line 20 col 11
// CHECK-NEXT:    bc 354: line 20 col 11
// CHECK-NEXT:  0x007b  end of debug source table
