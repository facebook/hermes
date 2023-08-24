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
// CHECK-NEXT:  String count: 11
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
// CHECK-NEXT:s7[ASCII, 144..149]: global
// CHECK-NEXT:i8[ASCII, 150..162] #4CCB9499: simpleAsyncFE
// CHECK-NEXT:i9[ASCII, 163..173] #FD482E4F: simpleAwait
// CHECK-NEXT:i10[ASCII, 174..185] #EB416734: simpleReturn

// CHECK:Function Source Table:
// CHECK-NEXT:  Function ID 3 -> s0
// CHECK-NEXT:  Function ID 6 -> s0
// CHECK-NEXT:  Function ID 9 -> s0

// CHECK:Function<global>(1 params, 11 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "simpleAsyncFE"
// CHECK-NEXT:    DeclareGlobalVar  "simpleReturn"
// CHECK-NEXT:    DeclareGlobalVar  "simpleAwait"
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    CreateAsyncClosure r1, r0, NCFunction<simpleReturn>
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutById           r2, r1, 1, "simpleReturn"
// CHECK-NEXT:    CreateAsyncClosure r3, r0, NCFunction<simpleAwait>
// CHECK-NEXT:    GetGlobalObject   r4
// CHECK-NEXT:    PutById           r4, r3, 2, "simpleAwait"
// CHECK-NEXT:    LoadConstUndefined r6
// CHECK-NEXT:    Mov               r5, r6
// CHECK-NEXT:    CreateAsyncClosure r7, r0, NCFunction<simpleAsyncFE>
// CHECK-NEXT:    GetGlobalObject   r8
// CHECK-NEXT:    PutById           r8, r7, 3, "simpleAsyncFE"
// CHECK-NEXT:    Mov               r9, r5
// CHECK-NEXT:    Ret               r9

// CHECK:NCFunction<simpleReturn>(1 params, 20 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x001f, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadThisNS        r1
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Mov               r2, r3
// CHECK-NEXT:    CreateGeneratorClosure r4, r0, NCFunction<?anon_0_simpleReturn>
// CHECK-NEXT:    GetBuiltinClosure r5, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArguments    r2
// CHECK-NEXT:    Mov               r6, r2
// CHECK-NEXT:    LoadConstUndefined r7
// CHECK-NEXT:    Mov               r13, r7
// CHECK-NEXT:    Mov               r12, r4
// CHECK-NEXT:    Mov               r11, r1
// CHECK-NEXT:    Mov               r10, r6
// CHECK-NEXT:    Call              r8, r5, 4
// CHECK-NEXT:    Ret               r8

// CHECK:NCFunction<?anon_0_simpleReturn>(1 params, 3 registers, 0 symbols):
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    CreateGenerator   r1, r0, Function<?anon_0_?anon_0_simpleReturn>
// CHECK-NEXT:    Ret               r1

// CHECK:Function<?anon_0_?anon_0_simpleReturn>(1 params, 4 registers, 0 symbols):
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    ResumeGenerator   r1, r0
// CHECK-NEXT:    Mov               r2, r0
// CHECK-NEXT:    JmpTrue           L1, r2
// CHECK-NEXT:    LoadConstUInt8    r0, 1
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<simpleAwait>(1 params, 21 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x002c, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadThisNS        r1
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Mov               r2, r3
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r4
// CHECK-NEXT:    CreateGeneratorClosure r5, r0, NCFunction<?anon_0_simpleAwait>
// CHECK-NEXT:    GetBuiltinClosure r6, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArguments    r2
// CHECK-NEXT:    Mov               r7, r2
// CHECK-NEXT:    LoadConstUndefined r8
// CHECK-NEXT:    Mov               r14, r8
// CHECK-NEXT:    Mov               r13, r5
// CHECK-NEXT:    Mov               r12, r1
// CHECK-NEXT:    Mov               r11, r7
// CHECK-NEXT:    Call              r9, r6, 4
// CHECK-NEXT:    Ret               r9

// CHECK:NCFunction<?anon_0_simpleAwait>(1 params, 4 registers, 1 symbols):
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    CreateGenerator   r2, r0, Function<?anon_0_?anon_0_simpleAwait>
// CHECK-NEXT:    Ret               r2

// CHECK:Function<?anon_0_?anon_0_simpleAwait>(1 params, 6 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0039, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    ResumeGenerator   r2, r1
// CHECK-NEXT:    Mov               r3, r1
// CHECK-NEXT:    JmpTrue           L1, r3
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    LoadConstUInt8    r4, 2
// CHECK-NEXT:    SaveGenerator     L2
// CHECK-NEXT:    Ret               r4
// CHECK-NEXT:L2:
// CHECK-NEXT:    ResumeGenerator   r1, r3
// CHECK-NEXT:    Mov               r4, r3
// CHECK-NEXT:    JmpTrue           L3, r4
// CHECK-NEXT:    StoreToEnvironment r0, 0, r1
// CHECK-NEXT:    LoadFromEnvironment r4, r0, 0
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r4
// CHECK-NEXT:L3:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r2

// CHECK:NCFunction<simpleAsyncFE>(1 params, 21 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x004f, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadThisNS        r1
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Mov               r2, r3
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r4
// CHECK-NEXT:    CreateGeneratorClosure r5, r0, NCFunction<?anon_0_simpleAsyncFE>
// CHECK-NEXT:    GetBuiltinClosure r6, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArguments    r2
// CHECK-NEXT:    Mov               r7, r2
// CHECK-NEXT:    LoadConstUndefined r8
// CHECK-NEXT:    Mov               r14, r8
// CHECK-NEXT:    Mov               r13, r5
// CHECK-NEXT:    Mov               r12, r1
// CHECK-NEXT:    Mov               r11, r7
// CHECK-NEXT:    Call              r9, r6, 4
// CHECK-NEXT:    Ret               r9

// CHECK:NCFunction<?anon_0_simpleAsyncFE>(1 params, 4 registers, 1 symbols):
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    CreateGenerator   r2, r0, Function<?anon_0_?anon_0_simpleAsyncFE>
// CHECK-NEXT:    Ret               r2

// CHECK:Function<?anon_0_?anon_0_simpleAsyncFE>(1 params, 6 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x005c, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    ResumeGenerator   r2, r1
// CHECK-NEXT:    Mov               r3, r1
// CHECK-NEXT:    JmpTrue           L1, r3
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    LoadConstUInt8    r4, 2
// CHECK-NEXT:    SaveGenerator     L2
// CHECK-NEXT:    Ret               r4
// CHECK-NEXT:L2:
// CHECK-NEXT:    ResumeGenerator   r1, r3
// CHECK-NEXT:    Mov               r4, r3
// CHECK-NEXT:    JmpTrue           L3, r4
// CHECK-NEXT:    StoreToEnvironment r0, 0, r1
// CHECK-NEXT:    LoadFromEnvironment r4, r0, 0
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r4
// CHECK-NEXT:L3:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r2

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}async-function.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 24: line 10 col 1 scope offset 0x0000 env none
// CHECK-NEXT:    bc 37: line 10 col 1 scope offset 0x0000 env none
// CHECK-NEXT:    bc 55: line 19 col 19 scope offset 0x0000 env none
// CHECK-NEXT:  0x001f  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 36: line 10 col 1 scope offset 0x0000 env none
// CHECK-NEXT:  0x002c  function idx 4, starts at line 14 col 1
// CHECK-NEXT:    bc 42: line 14 col 1 scope offset 0x0000 env none
// CHECK-NEXT:  0x0039  function idx 6, starts at line 14 col 1
// CHECK-NEXT:    bc 21: line 15 col 11 scope offset 0x0000 env none
// CHECK-NEXT:    bc 25: line 15 col 11 scope offset 0x0000 env none
// CHECK-NEXT:  0x004f  function idx 7, starts at line 19 col 21
// CHECK-NEXT:    bc 42: line 19 col 21 scope offset 0x0000 env none
// CHECK-NEXT:  0x005c  function idx 9, starts at line 19 col 21
// CHECK-NEXT:    bc 21: line 20 col 11 scope offset 0x0000 env none
// CHECK-NEXT:    bc 25: line 20 col 11 scope offset 0x0000 env none
// CHECK-NEXT:  0x0072  end of debug source table

// CHECK:Debug scope descriptor table:
// CHECK-NEXT:  0x0000  lexical parent:   none, flags:    , variable count: 0
// CHECK-NEXT:  0x0003  end of debug scope descriptor table

// CHECK:Textified callees table:
// CHECK-NEXT:  0x0000  entries: 0
// CHECK-NEXT:  0x0001  end of textified callees table

// CHECK:Debug string table:
// CHECK-NEXT:  0x0000  end of debug string table
