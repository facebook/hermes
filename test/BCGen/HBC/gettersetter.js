/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheckOrRegen --match-full-lines %s

var obj = {
  get b() {},
  set b(x) {},
  get c() {},
  set d(x) {},
};

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 5
// CHECK-NEXT:  String count: 9
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..4]: get b
// CHECK-NEXT:s1[ASCII, 4..4]: b
// CHECK-NEXT:s2[ASCII, 5..9]: get c
// CHECK-NEXT:s3[ASCII, 9..9]: c
// CHECK-NEXT:s4[ASCII, 10..14]: set d
// CHECK-NEXT:s5[ASCII, 14..14]: d
// CHECK-NEXT:s6[ASCII, 15..20]: global
// CHECK-NEXT:s7[ASCII, 21..25]: set b
// CHECK-NEXT:i8[ASCII, 26..28] #DC53DBCF: obj

// CHECK:Function<global>(1 params, 5 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateEnvironment r1
// CHECK-NEXT:    DeclareGlobalVar  "obj"
// CHECK-NEXT:    NewObject         r2
// CHECK-NEXT:    CreateClosure     r4, r1, Function<get b>
// CHECK-NEXT:    CreateClosure     r3, r1, Function<set b>
// CHECK-NEXT:    LoadConstString   r0, "b"
// CHECK-NEXT:    PutOwnGetterSetterByVal r2, r0, r4, r3, 1
// CHECK-NEXT:    CreateClosure     r4, r1, Function<get c>
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    LoadConstString   r3, "c"
// CHECK-NEXT:    PutOwnGetterSetterByVal r2, r3, r4, r0, 1
// CHECK-NEXT:    CreateClosure     r3, r1, Function<set d>
// CHECK-NEXT:    LoadConstString   r1, "d"
// CHECK-NEXT:    PutOwnGetterSetterByVal r2, r1, r0, r3, 1
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    PutByIdLoose      r1, r2, 1, "obj"
// CHECK-NEXT:    Ret               r0

// CHECK:Function<get b>(1 params, 1 registers, 0 symbols):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<set b>(2 params, 1 registers, 0 symbols):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<get c>(1 params, 1 registers, 0 symbols):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<set d>(2 params, 1 registers, 0 symbols):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}gettersetter.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 2: line 10 col 1
// CHECK-NEXT:    bc 23: line 10 col 11
// CHECK-NEXT:    bc 40: line 10 col 11
// CHECK-NEXT:    bc 55: line 10 col 11
// CHECK-NEXT:    bc 63: line 10 col 9
// CHECK-NEXT:  0x0013  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
