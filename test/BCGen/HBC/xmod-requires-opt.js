/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

// This tests simulates the Metro requires tranform, and shows that
// requires calls in a module are compiled to a bytecode instruction.

// The important case is the call in the module factory function
// modFact1, which does "require(0)".  This calls should be
// translated into the CallRequire bytecode instruction.

function require(modIdx) {
  switch (modIdx) {
  case 0: {
    return $SHBuiltin.moduleFactory(
      0,
      function modFact0(global, require, module, exports) {
        function bar() {
          return 17;
        }
        exports.bar = bar;
        return exports;
      })(undefined, require, mod, exports);
  }
  case 1: {
    return $SHBuiltin.moduleFactory(
      1,
      function modFact1(global, require, module, exports) {
        exports.x = require(0).bar();
        return exports;
      })(undefined, require, mod, exports);
  }
  default:
  }
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
// CHECK-NEXT:  StringSwitchImm count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..5]: global
// CHECK-NEXT:s1[ASCII, 6..13]: modFact0
// CHECK-NEXT:s2[ASCII, 14..21]: modFact1
// CHECK-NEXT:i3[ASCII, 6..8] #5D0193F1: mod
// CHECK-NEXT:i4[ASCII, 22..24] #9B85A7ED: bar
// CHECK-NEXT:i5[ASCII, 24..30] #EB75CA32: require
// CHECK-NEXT:i6[ASCII, 30..36] #C765D706: exports
// CHECK-NEXT:i7[ASCII, 37..37] #0001E7F9: x

// CHECK:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "require"
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    CreateClosure     r1, r0, Function<require>
// CHECK-NEXT:    PutByIdLoose      r2, r1, 0, "require"
// CHECK-NEXT:    Ret               r0

// CHECK:Function<require>(2 params, 18 registers, 1 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x000b, lexical 0x0000
// CHECK-NEXT:    LoadParam         r2, 1
// CHECK-NEXT:    LoadConstZero     r0
// CHECK-NEXT:    JStrictEqual      L1, r0, r2
// CHECK-NEXT:    LoadConstUInt8    r0, 1
// CHECK-NEXT:    JStrictEqual      L2, r0, r2
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Ret               r1
// CHECK-NEXT:L2:
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    GetByIdShort      r8, r2, 0, "require"
// CHECK-NEXT:    TryGetById        r7, r2, 1, "mod"
// CHECK-NEXT:    TryGetById        r2, r2, 2, "exports"
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    CreateClosure     r3, r1, Function<modFact1>
// CHECK-NEXT:    LoadConstUndefined r10
// CHECK-NEXT:    LoadConstUndefined r9
// CHECK-NEXT:    Mov               r6, r2
// CHECK-NEXT:    Call              r3, r3, 5
// CHECK-NEXT:    Ret               r2
// CHECK-NEXT:L1:
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    GetByIdShort      r8, r2, 0, "require"
// CHECK-NEXT:    TryGetById        r7, r2, 1, "mod"
// CHECK-NEXT:    TryGetById        r2, r2, 2, "exports"
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    CreateClosure     r3, r1, Function<modFact0>
// CHECK-NEXT:    LoadConstUndefined r10
// CHECK-NEXT:    LoadConstUndefined r9
// CHECK-NEXT:    Mov               r6, r2
// CHECK-NEXT:    Call              r3, r3, 5
// CHECK-NEXT:    Ret               r2

// CHECK:Function<modFact0>(5 params, 3 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0029, lexical 0x0000
// CHECK-NEXT:    LoadParam         r1, 4
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    CreateClosure     r2, r0, Function<bar>
// CHECK-NEXT:    PutByIdLoose      r1, r2, 0, "bar"
// CHECK-NEXT:    Ret               r1

// CHECK:Function<modFact1>(5 params, 13 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0031, lexical 0x0000
// CHECK-NEXT:    LoadParam         r1, 4
// CHECK-NEXT:    LoadConstUndefined r5
// CHECK-NEXT:    LoadParam         r2, 2
// CHECK-NEXT:    CallRequire       r3, r2, 0
// CHECK-NEXT:    GetByIdShort      r2, r3, 0, "bar"
// CHECK-NEXT:    Call1             r2, r2, r3
// CHECK-NEXT:    PutByIdLoose      r1, r2, 0, "x"
// CHECK-NEXT:    Ret               r1

// CHECK:Function<bar>(1 params, 1 registers, 1 numbers, 0 non-pointers):
// CHECK-NEXT:    LoadConstUInt8    r0, 17
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}xmod-requires-opt.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 17 col 1
// CHECK-NEXT:    bc 0: line 17 col 1
// CHECK-NEXT:    bc 14: line 17 col 1
// CHECK-NEXT:  0x000b  function idx 1, starts at line 17 col 1
// CHECK-NEXT:    bc 22: line 36 col 21
// CHECK-NEXT:    bc 27: line 36 col 30
// CHECK-NEXT:    bc 33: line 36 col 35
// CHECK-NEXT:    bc 53: line 36 col 9
// CHECK-NEXT:    bc 61: line 28 col 21
// CHECK-NEXT:    bc 66: line 28 col 30
// CHECK-NEXT:    bc 72: line 28 col 35
// CHECK-NEXT:    bc 92: line 28 col 9
// CHECK-NEXT:  0x0029  function idx 2, starts at line 22 col 7
// CHECK-NEXT:    bc 10: line 26 col 21
// CHECK-NEXT:  0x0031  function idx 3, starts at line 33 col 7
// CHECK-NEXT:    bc 8: line 34 col 28
// CHECK-NEXT:    bc 15: line 34 col 35
// CHECK-NEXT:    bc 20: line 34 col 35
// CHECK-NEXT:    bc 24: line 34 col 19
// CHECK-NEXT:  0x0042  end of debug source table
