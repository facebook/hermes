/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -Xdump-functions=global -dump-bytecode -O -strict %s | %FileCheckOrRegen --match-full-lines --check-prefix=STRICT %s
// RUN: %hermes -target=HBC -Xdump-functions=global -dump-bytecode -O -non-strict %s | %FileCheckOrRegen --match-full-lines --check-prefix=NONSTRICT %s

var x = 5;
foo(x);
y = x;

// Auto-generated content below. Please do not modify manually.

// STRICT:Bytecode File Information:
// STRICT-NEXT:  Bytecode version number: {{.*}}
// STRICT-NEXT:  Source hash: {{.*}}
// STRICT-NEXT:  Function count: 1
// STRICT-NEXT:  String count: 4
// STRICT-NEXT:  BigInt count: 0
// STRICT-NEXT:  String Kind Entry count: 2
// STRICT-NEXT:  RegExp count: 0
// STRICT-NEXT:  StringSwitchImm count: 0
// STRICT-NEXT:  Segment ID: 0
// STRICT-NEXT:  CommonJS module count: 0
// STRICT-NEXT:  CommonJS module count (static): 0
// STRICT-NEXT:  Function source count: 0
// STRICT-NEXT:  Bytecode options:
// STRICT-NEXT:    staticBuiltins: 0
// STRICT-NEXT:    cjsModulesStaticallyResolved: 0

// STRICT:Global String Table:
// STRICT-NEXT:s0[ASCII, 0..5]: global
// STRICT-NEXT:i1[ASCII, 6..8] #9290584E: foo
// STRICT-NEXT:i2[ASCII, 9..9] #0001E7F9: x
// STRICT-NEXT:i3[ASCII, 10..10] #0001E3E8: y

// STRICT:Function<global>(1 params, 14 registers, 1 numbers, 1 non-pointers):
// STRICT-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// STRICT-NEXT:    DeclareGlobalVar  "x"
// STRICT-NEXT:    GetGlobalObject   r3
// STRICT-NEXT:    LoadConstUInt8    r0, 5
// STRICT-NEXT:    PutByIdStrict     r3, r0, 0, "x"
// STRICT-NEXT:    TryGetById        r4, r3, 0, "foo"
// STRICT-NEXT:    GetByIdShort      r2, r3, 1, "x"
// STRICT-NEXT:    LoadConstUndefined r1
// STRICT-NEXT:    Call2             r2, r4, r1, r2
// STRICT-NEXT:    GetByIdShort      r2, r3, 1, "x"
// STRICT-NEXT:    TryPutByIdStrict  r3, r2, 1, "y"
// STRICT-NEXT:    Ret               r2

// STRICT:Debug filename table:
// STRICT-NEXT:  0: {{.*}}globals.js

// STRICT:Debug file table:
// STRICT-NEXT:  source table offset 0x0000: filename id 0

// STRICT:Debug source table:
// STRICT-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// STRICT-NEXT:    bc 0: line 11 col 1
// STRICT-NEXT:    bc 10: line 11 col 7
// STRICT-NEXT:    bc 16: line 12 col 1
// STRICT-NEXT:    bc 22: line 12 col 5
// STRICT-NEXT:    bc 29: line 12 col 4
// STRICT-NEXT:    bc 34: line 13 col 5
// STRICT-NEXT:    bc 39: line 13 col 3
// STRICT-NEXT:  0x0019  end of debug source table

// NONSTRICT:Bytecode File Information:
// NONSTRICT-NEXT:  Bytecode version number: {{.*}}
// NONSTRICT-NEXT:  Source hash: {{.*}}
// NONSTRICT-NEXT:  Function count: 1
// NONSTRICT-NEXT:  String count: 4
// NONSTRICT-NEXT:  BigInt count: 0
// NONSTRICT-NEXT:  String Kind Entry count: 2
// NONSTRICT-NEXT:  RegExp count: 0
// NONSTRICT-NEXT:  StringSwitchImm count: 0
// NONSTRICT-NEXT:  Segment ID: 0
// NONSTRICT-NEXT:  CommonJS module count: 0
// NONSTRICT-NEXT:  CommonJS module count (static): 0
// NONSTRICT-NEXT:  Function source count: 0
// NONSTRICT-NEXT:  Bytecode options:
// NONSTRICT-NEXT:    staticBuiltins: 0
// NONSTRICT-NEXT:    cjsModulesStaticallyResolved: 0

// NONSTRICT:Global String Table:
// NONSTRICT-NEXT:s0[ASCII, 0..5]: global
// NONSTRICT-NEXT:i1[ASCII, 6..8] #9290584E: foo
// NONSTRICT-NEXT:i2[ASCII, 9..9] #0001E7F9: x
// NONSTRICT-NEXT:i3[ASCII, 10..10] #0001E3E8: y

// NONSTRICT:Function<global>(1 params, 14 registers, 1 numbers, 1 non-pointers):
// NONSTRICT-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// NONSTRICT-NEXT:    DeclareGlobalVar  "x"
// NONSTRICT-NEXT:    GetGlobalObject   r3
// NONSTRICT-NEXT:    LoadConstUInt8    r0, 5
// NONSTRICT-NEXT:    PutByIdLoose      r3, r0, 0, "x"
// NONSTRICT-NEXT:    TryGetById        r4, r3, 0, "foo"
// NONSTRICT-NEXT:    GetByIdShort      r2, r3, 1, "x"
// NONSTRICT-NEXT:    LoadConstUndefined r1
// NONSTRICT-NEXT:    Call2             r2, r4, r1, r2
// NONSTRICT-NEXT:    GetByIdShort      r2, r3, 1, "x"
// NONSTRICT-NEXT:    PutByIdLoose      r3, r2, 1, "y"
// NONSTRICT-NEXT:    Ret               r2

// NONSTRICT:Debug filename table:
// NONSTRICT-NEXT:  0: {{.*}}globals.js

// NONSTRICT:Debug file table:
// NONSTRICT-NEXT:  source table offset 0x0000: filename id 0

// NONSTRICT:Debug source table:
// NONSTRICT-NEXT:  0x0000  function idx 0, starts at line 11 col 1
// NONSTRICT-NEXT:    bc 0: line 11 col 1
// NONSTRICT-NEXT:    bc 10: line 11 col 7
// NONSTRICT-NEXT:    bc 16: line 12 col 1
// NONSTRICT-NEXT:    bc 22: line 12 col 5
// NONSTRICT-NEXT:    bc 29: line 12 col 4
// NONSTRICT-NEXT:    bc 34: line 13 col 5
// NONSTRICT-NEXT:    bc 39: line 13 col 3
// NONSTRICT-NEXT:  0x0019  end of debug source table
