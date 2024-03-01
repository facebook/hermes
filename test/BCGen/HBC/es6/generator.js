/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheckOrRegen %s --match-full-lines

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
// CHECK-NEXT:  String count: 8
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
// CHECK-NEXT:s1[ASCII, 0..11]: ?anon_0_args
// CHECK-NEXT:s2[ASCII, 12..23]: ?anon_0_loop
// CHECK-NEXT:s3[ASCII, 24..35]: DONE LOOPING
// CHECK-NEXT:s4[ASCII, 36..41]: global
// CHECK-NEXT:i5[ASCII, 42..45] #50273FEB: args
// CHECK-NEXT:i6[ASCII, 46..49] #EFC200CF: loop
// CHECK-NEXT:i7[ASCII, 50..50] #0001E3E8: y

// CHECK:Function Source Table:
// CHECK-NEXT:  Function ID 3 -> s0
// CHECK-NEXT:  Function ID 4 -> s0

// CHECK:Function<global>(1 params, 9 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateFunctionEnvironment r0
// CHECK-NEXT:    DeclareGlobalVar  "loop"
// CHECK-NEXT:    DeclareGlobalVar  "args"
// CHECK-NEXT:    CreateGeneratorClosure r1, r0, NCFunction<loop>
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "loop"
// CHECK-NEXT:    CreateGeneratorClosure r3, r0, NCFunction<args>
// CHECK-NEXT:    GetGlobalObject   r4
// CHECK-NEXT:    PutByIdLoose      r4, r3, 2, "args"
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    Mov               r5, r6
// CHECK-NEXT:    Mov               r7, r5
// CHECK-NEXT:    Ret               r7

// CHECK:NCFunction<loop>(2 params, 4 registers, 0 symbols):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    CreateEnvironment r1, r0, 0
// CHECK-NEXT:    CreateGenerator   r2, r1, Function<?anon_0_loop>
// CHECK-NEXT:    Ret               r2

// CHECK:NCFunction<args>(1 params, 4 registers, 0 symbols):
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    CreateEnvironment r1, r0, 0
// CHECK-NEXT:    CreateGenerator   r2, r1, Function<?anon_0_args>
// CHECK-NEXT:    Ret               r2

// CHECK:Function<?anon_0_loop>(2 params, 9 registers, 2 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    ResumeGenerator   r1, r0
// CHECK-NEXT:    Mov               r2, r0
// CHECK-NEXT:    JmpTrue           L1, r2
// CHECK-NEXT:    GetParentEnvironment r0, 0
// CHECK-NEXT:    CreateEnvironment r2, r0, 2
// CHECK-NEXT:    LoadParam         r3, 1
// CHECK-NEXT:    StoreToEnvironment r2, 0, r3
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    StoreNPToEnvironment r2, 1, r4
// CHECK-NEXT:    LoadConstZero     r5
// CHECK-NEXT:    StoreNPToEnvironment r2, 1, r5
// CHECK-NEXT:    GetGlobalObject   r6
// CHECK-NEXT:    TryGetById        r7, r6, 1, "y"
// CHECK-NEXT:    JmpFalse          L2, r7
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadFromEnvironment r0, r2, 0
// CHECK-NEXT:    LoadFromEnvironment r3, r2, 1
// CHECK-NEXT:    ToNumeric         r4, r3
// CHECK-NEXT:    Inc               r5, r4
// CHECK-NEXT:    StoreToEnvironment r2, 1, r5
// CHECK-NEXT:    GetByVal          r6, r0, r4
// CHECK-NEXT:    SaveGenerator     L3
// CHECK-NEXT:    Ret               r6
// CHECK-NEXT:L3:
// CHECK-NEXT:    ResumeGenerator   r0, r7
// CHECK-NEXT:    Mov               r3, r7
// CHECK-NEXT:    JmpTrue           L4, r3
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    TryGetById        r4, r3, 1, "y"
// CHECK-NEXT:    JmpTrue           L5, r4
// CHECK-NEXT:L2:
// CHECK-NEXT:    LoadConstString   r3, "DONE LOOPING"
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r3
// CHECK-NEXT:L4:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r1

// CHECK:Function<?anon_0_args>(1 params, 8 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0026, lexical 0x0000
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    ResumeGenerator   r1, r0
// CHECK-NEXT:    Mov               r2, r0
// CHECK-NEXT:    JmpTrue           L1, r2
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r0, r2
// CHECK-NEXT:    GetParentEnvironment r3, 0
// CHECK-NEXT:    CreateEnvironment r4, r3, 0
// CHECK-NEXT:    LoadConstZero     r4
// CHECK-NEXT:    GetArgumentsPropByValLoose r5, r4, r0
// CHECK-NEXT:    SaveGenerator     L2
// CHECK-NEXT:    Ret               r5
// CHECK-NEXT:L2:
// CHECK-NEXT:    ResumeGenerator   r0, r6
// CHECK-NEXT:    Mov               r2, r6
// CHECK-NEXT:    JmpTrue           L3, r2
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L3:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r1

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}generator.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 2: line 10 col 1
// CHECK-NEXT:    bc 7: line 10 col 1
// CHECK-NEXT:    bc 19: line 10 col 1
// CHECK-NEXT:    bc 32: line 10 col 1
// CHECK-NEXT:  0x0010  function idx 3, starts at line 10 col 1
// CHECK-NEXT:    bc 41: line 12 col 10
// CHECK-NEXT:    bc 58: line 13 col 14
// CHECK-NEXT:    bc 68: line 13 col 12
// CHECK-NEXT:    bc 72: line 13 col 5
// CHECK-NEXT:    bc 76: line 13 col 5
// CHECK-NEXT:    bc 87: line 12 col 10
// CHECK-NEXT:  0x0026  function idx 4, starts at line 18 col 1
// CHECK-NEXT:    bc 27: line 19 col 18
// CHECK-NEXT:    bc 31: line 19 col 3
// CHECK-NEXT:    bc 35: line 19 col 3
// CHECK-NEXT:  0x0033  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
