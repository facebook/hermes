/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheckOrRegen --match-full-lines %s

function binary() {
  var x = foo(), y = foo(), z;
  z = x == y;
  z = x != y;
  z = x === y;
  z = x != y;
  z = x<y;
  z = x <= y;
  z = x>y;
  z = x >= y;
  z = x << y;
  z = x >> y;
  z = x >>> y;
  z = x + y;
  z = x - y;
  z = x * y;
  z = x / y;
  z = x % y;
  z = x | y;
  z = x ^ y;
  z = x & y;
  z = x in y;
  return x !== y;
}

function foo() { return; }

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 3
// CHECK-NEXT:  String count: 3
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
// CHECK-NEXT:s0[ASCII, 0..5]: global
// CHECK-NEXT:i1[ASCII, 6..11] #A5D4F6F9: binary
// CHECK-NEXT:i2[ASCII, 12..14] #9290584E: foo

// CHECK:Function<global>(1 params, 3 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateEnvironment r0
// CHECK-NEXT:    DeclareGlobalVar  "binary"
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateClosure     r2, r0, Function<binary>
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    PutByIdLoose      r1, r2, 1, "binary"
// CHECK-NEXT:    CreateClosure     r0, r0, Function<foo>
// CHECK-NEXT:    PutByIdLoose      r1, r0, 2, "foo"
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<binary>(1 params, 11 registers, 0 symbols):
// CHECK-NEXT:Offset in debug table: source 0x0010, lexical 0x0000
// CHECK-NEXT:    GetGlobalObject   r0
// CHECK-NEXT:    GetByIdShort      r1, r0, 1, "foo"
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Call1             r1, r1, r2
// CHECK-NEXT:    GetByIdShort      r0, r0, 1, "foo"
// CHECK-NEXT:    Call1             r0, r0, r2
// CHECK-NEXT:    Eq                r2, r1, r0
// CHECK-NEXT:    Neq               r2, r1, r0
// CHECK-NEXT:    Neq               r2, r1, r0
// CHECK-NEXT:    Less              r2, r1, r0
// CHECK-NEXT:    LessEq            r2, r1, r0
// CHECK-NEXT:    Greater           r2, r1, r0
// CHECK-NEXT:    GreaterEq         r2, r1, r0
// CHECK-NEXT:    LShift            r2, r1, r0
// CHECK-NEXT:    RShift            r2, r1, r0
// CHECK-NEXT:    URshift           r2, r1, r0
// CHECK-NEXT:    Add               r2, r1, r0
// CHECK-NEXT:    Sub               r2, r1, r0
// CHECK-NEXT:    Mul               r2, r1, r0
// CHECK-NEXT:    Div               r2, r1, r0
// CHECK-NEXT:    Mod               r2, r1, r0
// CHECK-NEXT:    BitOr             r2, r1, r0
// CHECK-NEXT:    BitXor            r2, r1, r0
// CHECK-NEXT:    BitAnd            r2, r1, r0
// CHECK-NEXT:    IsIn              r2, r1, r0
// CHECK-NEXT:    StrictNeq         r0, r1, r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<foo>(1 params, 1 registers, 0 symbols):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}binary.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 2: line 10 col 1
// CHECK-NEXT:    bc 7: line 10 col 1
// CHECK-NEXT:    bc 19: line 10 col 1
// CHECK-NEXT:    bc 30: line 10 col 1
// CHECK-NEXT:  0x0010  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 2: line 11 col 11
// CHECK-NEXT:    bc 9: line 11 col 14
// CHECK-NEXT:    bc 13: line 11 col 22
// CHECK-NEXT:    bc 18: line 11 col 25
// CHECK-NEXT:    bc 22: line 12 col 7
// CHECK-NEXT:    bc 26: line 13 col 7
// CHECK-NEXT:    bc 30: line 15 col 7
// CHECK-NEXT:    bc 34: line 16 col 7
// CHECK-NEXT:    bc 38: line 17 col 7
// CHECK-NEXT:    bc 42: line 18 col 7
// CHECK-NEXT:    bc 46: line 19 col 7
// CHECK-NEXT:    bc 50: line 20 col 7
// CHECK-NEXT:    bc 54: line 21 col 7
// CHECK-NEXT:    bc 58: line 22 col 7
// CHECK-NEXT:    bc 62: line 23 col 7
// CHECK-NEXT:    bc 66: line 24 col 7
// CHECK-NEXT:    bc 70: line 25 col 7
// CHECK-NEXT:    bc 74: line 26 col 7
// CHECK-NEXT:    bc 78: line 27 col 7
// CHECK-NEXT:    bc 82: line 28 col 7
// CHECK-NEXT:    bc 86: line 29 col 7
// CHECK-NEXT:    bc 90: line 30 col 7
// CHECK-NEXT:    bc 94: line 31 col 7
// CHECK-NEXT:  0x0059  end of debug source table

// CHECK:Debug lexical table:
// CHECK-NEXT:  0x0000  lexical parent: none, variable count: 0
// CHECK-NEXT:  0x0002  end of debug lexical table
