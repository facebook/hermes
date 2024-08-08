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

// CHECK:Function<global>(1 params, 4 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "simpleAsyncFE"
// CHECK-NEXT:    DeclareGlobalVar  "simpleReturn"
// CHECK-NEXT:    DeclareGlobalVar  "simpleAwait"
// CHECK-NEXT:    CreateEnvironment r2
// CHECK-NEXT:    CreateAsyncClosure r1, r2, NCFunction<simpleReturn>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutById           r3, r1, 1, "simpleReturn"
// CHECK-NEXT:    CreateAsyncClosure r1, r2, NCFunction<simpleAwait>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutById           r3, r1, 2, "simpleAwait"
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    Mov               r1, r3
// CHECK-NEXT:    CreateAsyncClosure r2, r2, NCFunction<simpleAsyncFE>
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutById           r3, r2, 3, "simpleAsyncFE"
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<simpleReturn>(1 params, 16 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x001f, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    CreateEnvironment r3
// CHECK-NEXT:    LoadThisNS        r4
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Mov               r5, r1
// CHECK-NEXT:    CreateGeneratorClosure r3, r3, NCFunction<?anon_0_simpleReturn>
// CHECK-NEXT:    GetBuiltinClosure r1, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArguments    r5
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r9, r2
// CHECK-NEXT:    Mov               r8, r3
// CHECK-NEXT:    Mov               r7, r4
// CHECK-NEXT:    Mov               r6, r5
// CHECK-NEXT:    Call              r1, r1, 4
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<?anon_0_simpleReturn>(1 params, 2 registers, 0 symbols):
// CHECK-NEXT:    CreateEnvironment r1
// CHECK-NEXT:    CreateGenerator   r1, r1, Function<?anon_0_?anon_0_simpleReturn>
// CHECK-NEXT:    Ret               r1

// CHECK:Function<?anon_0_?anon_0_simpleReturn>(1 params, 3 registers, 0 symbols):
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    ResumeGenerator   r1, r2
// CHECK-NEXT:    JmpTrue           L1, r2
// CHECK-NEXT:    LoadConstUInt8    r2, 1
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<simpleAwait>(1 params, 16 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x002c, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    CreateEnvironment r3
// CHECK-NEXT:    LoadThisNS        r4
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Mov               r5, r1
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r3, 0, r1
// CHECK-NEXT:    CreateGeneratorClosure r3, r3, NCFunction<?anon_0_simpleAwait>
// CHECK-NEXT:    GetBuiltinClosure r1, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArguments    r5
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r9, r2
// CHECK-NEXT:    Mov               r8, r3
// CHECK-NEXT:    Mov               r7, r4
// CHECK-NEXT:    Mov               r6, r5
// CHECK-NEXT:    Call              r1, r1, 4
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<?anon_0_simpleAwait>(1 params, 3 registers, 1 symbols):
// CHECK-NEXT:    CreateEnvironment r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 0, r2
// CHECK-NEXT:    CreateGenerator   r1, r1, Function<?anon_0_?anon_0_simpleAwait>
// CHECK-NEXT:    Ret               r1

// CHECK:Function<?anon_0_?anon_0_simpleAwait>(1 params, 6 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0039, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    CreateEnvironment r4
// CHECK-NEXT:    ResumeGenerator   r1, r3
// CHECK-NEXT:    JmpTrue           L1, r3
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    StoreNPToEnvironment r4, 0, r3
// CHECK-NEXT:    LoadConstUInt8    r3, 2
// CHECK-NEXT:    SaveGenerator     L2
// CHECK-NEXT:    Ret               r3
// CHECK-NEXT:L2:
// CHECK-NEXT:    ResumeGenerator   r2, r5
// CHECK-NEXT:    Mov               r3, r5
// CHECK-NEXT:    JmpTrue           L3, r3
// CHECK-NEXT:    StoreToEnvironment r4, 0, r2
// CHECK-NEXT:    LoadFromEnvironment r3, r4, 0
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r3
// CHECK-NEXT:L3:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<simpleAsyncFE>(1 params, 16 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x004f, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    CreateEnvironment r3
// CHECK-NEXT:    LoadThisNS        r4
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Mov               r5, r1
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r3, 0, r1
// CHECK-NEXT:    CreateGeneratorClosure r3, r3, NCFunction<?anon_0_simpleAsyncFE>
// CHECK-NEXT:    GetBuiltinClosure r1, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArguments    r5
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r9, r2
// CHECK-NEXT:    Mov               r8, r3
// CHECK-NEXT:    Mov               r7, r4
// CHECK-NEXT:    Mov               r6, r5
// CHECK-NEXT:    Call              r1, r1, 4
// CHECK-NEXT:    Ret               r1

// CHECK:NCFunction<?anon_0_simpleAsyncFE>(1 params, 3 registers, 1 symbols):
// CHECK-NEXT:    CreateEnvironment r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    StoreNPToEnvironment r1, 0, r2
// CHECK-NEXT:    CreateGenerator   r1, r1, Function<?anon_0_?anon_0_simpleAsyncFE>
// CHECK-NEXT:    Ret               r1

// CHECK:Function<?anon_0_?anon_0_simpleAsyncFE>(1 params, 6 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x005c, scope 0x0000, textified callees 0x0000
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    CreateEnvironment r4
// CHECK-NEXT:    ResumeGenerator   r1, r3
// CHECK-NEXT:    JmpTrue           L1, r3
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    StoreNPToEnvironment r4, 0, r3
// CHECK-NEXT:    LoadConstUInt8    r3, 2
// CHECK-NEXT:    SaveGenerator     L2
// CHECK-NEXT:    Ret               r3
// CHECK-NEXT:L2:
// CHECK-NEXT:    ResumeGenerator   r2, r5
// CHECK-NEXT:    Mov               r3, r5
// CHECK-NEXT:    JmpTrue           L3, r3
// CHECK-NEXT:    StoreToEnvironment r4, 0, r2
// CHECK-NEXT:    LoadFromEnvironment r3, r4, 0
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r3
// CHECK-NEXT:L3:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r1

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
// CHECK-NEXT:    bc 33: line 10 col 1 scope offset 0x0000 env none
// CHECK-NEXT:  0x002c  function idx 4, starts at line 14 col 1
// CHECK-NEXT:    bc 39: line 14 col 1 scope offset 0x0000 env none
// CHECK-NEXT:  0x0039  function idx 6, starts at line 14 col 1
// CHECK-NEXT:    bc 18: line 15 col 11 scope offset 0x0000 env none
// CHECK-NEXT:    bc 22: line 15 col 11 scope offset 0x0000 env none
// CHECK-NEXT:  0x004f  function idx 7, starts at line 19 col 21
// CHECK-NEXT:    bc 39: line 19 col 21 scope offset 0x0000 env none
// CHECK-NEXT:  0x005c  function idx 9, starts at line 19 col 21
// CHECK-NEXT:    bc 18: line 20 col 11 scope offset 0x0000 env none
// CHECK-NEXT:    bc 22: line 20 col 11 scope offset 0x0000 env none
// CHECK-NEXT:  0x0072  end of debug source table

// CHECK:Debug scope descriptor table:
// CHECK-NEXT:  0x0000  lexical parent:   none, flags:    , variable count: 0
// CHECK-NEXT:  0x0003  end of debug scope descriptor table

// CHECK:Textified callees table:
// CHECK-NEXT:  0x0000  entries: 0
// CHECK-NEXT:  0x0001  end of textified callees table

// CHECK:Debug string table:
// CHECK-NEXT:  0x0000  end of debug string table
