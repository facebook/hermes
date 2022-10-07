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
// CHECK-NEXT:  Bytecode version number: 90
// CHECK-NEXT:  Source hash: 0000000000000000000000000000000000000000
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
// CHECK-NEXT:  Function ID 2 -> s0
// CHECK-NEXT:  Function ID 4 -> s0

// CHECK:Function<global>(1 params, 8 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "loop"
// CHECK-NEXT:    DeclareGlobalVar  "args"
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    CreateGeneratorClosure r3, r0, NCFunction<loop>
// CHECK-NEXT:    PutById           r1, r3, 1, "loop"
// CHECK-NEXT:    CreateGeneratorClosure r4, r0, NCFunction<args>
// CHECK-NEXT:    PutById           r1, r4, 2, "args"
// CHECK-NEXT:    Mov               r5, r2
// CHECK-NEXT:    Mov               r6, r5
// CHECK-NEXT:    Ret               r6

// CHECK:NCFunction<loop>(2 params, 4 registers, 1 symbols):
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    CreateGenerator   r2, r0, Function<?anon_0_loop>
// CHECK-NEXT:    Ret               r2

// CHECK:Function<?anon_0_loop>(2 params, 14 registers, 2 symbols):
// CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadParam         r1, 1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    LoadConstZero     r3
// CHECK-NEXT:    LoadConstString   r4, "DONE LOOPING"
// CHECK-NEXT:    GetGlobalObject   r5
// CHECK-NEXT:    ResumeGenerator   r7, r6
// CHECK-NEXT:    Mov               r8, r6
// CHECK-NEXT:    JmpTrue           L1, r8
// CHECK-NEXT:    StoreToEnvironment r0, 0, r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 1, r2
// CHECK-NEXT:    StoreNPToEnvironment r0, 1, r3
// CHECK-NEXT:    TryGetById        r6, r5, 1, "y"
// CHECK-NEXT:    JmpFalse          L2, r6
// CHECK-NEXT:L5:
// CHECK-NEXT:    LoadFromEnvironment r6, r0, 0
// CHECK-NEXT:    LoadFromEnvironment r8, r0, 1
// CHECK-NEXT:    ToNumeric         r9, r8
// CHECK-NEXT:    Inc               r10, r9
// CHECK-NEXT:    StoreToEnvironment r0, 1, r10
// CHECK-NEXT:    GetByVal          r11, r6, r9
// CHECK-NEXT:    SaveGenerator     L3
// CHECK-NEXT:    Ret               r11
// CHECK-NEXT:L3:
// CHECK-NEXT:    ResumeGenerator   r6, r12
// CHECK-NEXT:    Mov               r8, r12
// CHECK-NEXT:    JmpTrue           L4, r8
// CHECK-NEXT:    TryGetById        r8, r5, 1, "y"
// CHECK-NEXT:    JmpTrue           L5, r8
// CHECK-NEXT:L2:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r4
// CHECK-NEXT:L4:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r6
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r7

// CHECK:NCFunction<args>(1 params, 3 registers, 0 symbols):
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    CreateGenerator   r1, r0, Function<?anon_0_args>
// CHECK-NEXT:    Ret               r1

// CHECK:Function<?anon_0_args>(1 params, 7 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0020, lexical 0x0000
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    LoadConstZero     r1
// CHECK-NEXT:    ResumeGenerator   r3, r2
// CHECK-NEXT:    Mov               r4, r2
// CHECK-NEXT:    JmpTrue           L1, r4
// CHECK-NEXT:    Mov               r2, r0
// CHECK-NEXT:    GetArgumentsPropByVal r4, r1, r2
// CHECK-NEXT:    SaveGenerator     L2
// CHECK-NEXT:    Ret               r4
// CHECK-NEXT:L2:
// CHECK-NEXT:    ResumeGenerator   r2, r5
// CHECK-NEXT:    Mov               r4, r5
// CHECK-NEXT:    JmpTrue           L3, r4
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L3:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r3

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}generator.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 21: line 10 col 1
// CHECK-NEXT:    bc 32: line 10 col 1
// CHECK-NEXT:  0x000a  function idx 2, starts at line 10 col 1
// CHECK-NEXT:    bc 37: line 12 col 10
// CHECK-NEXT:    bc 54: line 13 col 14
// CHECK-NEXT:    bc 64: line 13 col 12
// CHECK-NEXT:    bc 68: line 13 col 5
// CHECK-NEXT:    bc 72: line 13 col 5
// CHECK-NEXT:    bc 81: line 12 col 10
// CHECK-NEXT:  0x0020  function idx 4, starts at line 18 col 1
// CHECK-NEXT:    bc 19: line 19 col 18
// CHECK-NEXT:    bc 23: line 19 col 3
// CHECK-NEXT:    bc 27: line 19 col 3
// CHECK-NEXT:  0x002d  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
