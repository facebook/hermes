/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-bytecode %s | %FileCheck %s --match-full-lines

async function simpleReturn() {
  return 1;
}

// CHECK-LABEL:NCFunction<simpleReturn>(1 params, 19 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x{{.*}}, lexical 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadThisNS        r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r3, r2
// CHECK-NEXT:    CreateGeneratorClosure r4, r0, 2
// CHECK-NEXT:    GetBuiltinClosure r5, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArguments    r3
// CHECK-NEXT:    Mov               r6, r3
// CHECK-NEXT:    Mov               r12, r2
// CHECK-NEXT:    Mov               r11, r4
// CHECK-NEXT:    Mov               r10, r1
// CHECK-NEXT:    Mov               r9, r6
// CHECK-NEXT:    Call              r7, r5, 4
// CHECK-NEXT:    Ret               r7

// CHECK-LABEL:NCFunction<?anon_0_simpleReturn>(1 params, 3 registers, 0 symbols):
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    CreateGenerator   r1, r0, 3
// CHECK-NEXT:    Ret               r1

// CHECK-LABEL:Function<?anon_0_?anon_0_simpleReturn>(1 params, 6 registers, 0 symbols):
// CHECK-NEXT:    StartGenerator   
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUInt8    r0, 1
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    ResumeGenerator   r3, r2
// CHECK-NEXT:    Mov               r4, r2
// CHECK-NEXT:    JmpTrue           L1, r4
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r0
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r3

async function simpleAwait() {
  var x = await 2;
  return x;
}

// CHECK-LABEL:NCFunction<simpleAwait>(1 params, 19 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x{{.*}}, lexical 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadThisNS        r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r3, r2
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r2
// CHECK-NEXT:    CreateGeneratorClosure r4, r0, 5
// CHECK-NEXT:    GetBuiltinClosure r5, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArguments    r3
// CHECK-NEXT:    Mov               r6, r3
// CHECK-NEXT:    Mov               r12, r2
// CHECK-NEXT:    Mov               r11, r4
// CHECK-NEXT:    Mov               r10, r1
// CHECK-NEXT:    Mov               r9, r6
// CHECK-NEXT:    Call              r7, r5, 4
// CHECK-NEXT:    Ret               r7

// CHECK-LABEL:NCFunction<?anon_0_simpleAwait>(1 params, 4 registers, 1 symbols):
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    CreateGenerator   r2, r0, 6
// CHECK-NEXT:    Ret               r2

// CHECK-LABEL:Function<?anon_0_?anon_0_simpleAwait>(1 params, 8 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x{{.*}}, lexical 0x0000
// CHECK-NEXT:    StartGenerator
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    LoadConstUInt8    r2, 2
// CHECK-NEXT:    ResumeGenerator   r4, r3
// CHECK-NEXT:    Mov               r5, r3
// CHECK-NEXT:    JmpTrue           L1, r5
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    SaveGenerator     L2
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L2:
// CHECK-NEXT:    ResumeGenerator   r5, r3
// CHECK-NEXT:    Mov               r6, r3
// CHECK-NEXT:    JmpTrue           L3, r6
// CHECK-NEXT:    StoreToEnvironment r0, 0, r5
// CHECK-NEXT:    LoadFromEnvironment r6, r0, 0
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r6
// CHECK-NEXT:L3:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r5
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r4

var simpleAsyncFE = async function () {
  var x = await 2;
  return x;
}

// CHECK-LABEL:NCFunction<simpleAsyncFE>(1 params, 19 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x{{.*}}, lexical 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadThisNS        r1
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r3, r2
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r2
// CHECK-NEXT:    CreateGeneratorClosure r4, r0, 8
// CHECK-NEXT:    GetBuiltinClosure r5, "HermesBuiltin.spawnAsync"
// CHECK-NEXT:    ReifyArguments    r3
// CHECK-NEXT:    Mov               r6, r3
// CHECK-NEXT:    Mov               r12, r2
// CHECK-NEXT:    Mov               r11, r4
// CHECK-NEXT:    Mov               r10, r1
// CHECK-NEXT:    Mov               r9, r6
// CHECK-NEXT:    Call              r7, r5, 4
// CHECK-NEXT:    Ret               r7

// CHECK-LABEL:NCFunction<?anon_0_simpleAsyncFE>(1 params, 4 registers, 1 symbols):
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    CreateGenerator   r2, r0, 9
// CHECK-NEXT:    Ret               r2

// CHECK-LABEL:Function<?anon_0_?anon_0_simpleAsyncFE>(1 params, 8 registers, 1 symbols):
// CHECK-NEXT:Offset in debug table: source 0x{{.*}}, lexical 0x0000
// CHECK-NEXT:    StartGenerator   
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    LoadConstUInt8    r2, 2
// CHECK-NEXT:    ResumeGenerator   r4, r3
// CHECK-NEXT:    Mov               r5, r3
// CHECK-NEXT:    JmpTrue           L1, r5
// CHECK-NEXT:    StoreNPToEnvironment r0, 0, r1
// CHECK-NEXT:    SaveGenerator     L2
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L2:
// CHECK-NEXT:    ResumeGenerator   r5, r3
// CHECK-NEXT:    Mov               r6, r3
// CHECK-NEXT:    JmpTrue           L3, r6
// CHECK-NEXT:    StoreToEnvironment r0, 0, r5
// CHECK-NEXT:    LoadFromEnvironment r6, r0, 0
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r6
// CHECK-NEXT:L3:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r5
// CHECK-NEXT:L1:
// CHECK-NEXT:    CompleteGenerator
// CHECK-NEXT:    Ret               r4
