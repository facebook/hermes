/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode --pretty-disassemble -O %s | %FileCheckOrRegen --match-full-lines %s

function foo (a) {
    var sum = 0;
    while (--a)
        sum += a;
    print("This\nis един long Unicode string=", sum);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 2
// CHECK-NEXT:  String count: 4
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
// CHECK-NEXT:s1[UTF-16, 14..79]: \x54\x00\x68\x00\x69\x00\x73\x00\x0A\x00\x69\x00\x73\x00\x20\x00\x35\x04\x34\x04\x38\x04\x3D\x04\x20\x00\x6C\x00\x6F\x00\x6E\x00\x67\x00\x20\x00\x55\x00\x6E\x00\x69\x00\x63\x00\x6F\x00\x64\x00\x65\x00\x20\x00\x73\x00\x74\x00\x72\x00\x69\x00\x6E\x00\x67\x00\x3D\x00
// CHECK-NEXT:i2[ASCII, 6..8] #9290584E: foo
// CHECK-NEXT:i3[ASCII, 9..13] #A689F65B: print

// CHECK:Function<global>(1 params, 2 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateTopLevelEnvironment r1, 0
// CHECK-NEXT:    CreateClosure     r0, r1, Function<foo>
// CHECK-NEXT:    GetGlobalObject   r1
// CHECK-NEXT:    PutByIdLoose      r1, r0, 1, "foo"
// CHECK-NEXT:    LoadConstUndefined r1
// CHECK-NEXT:    Ret               r1

// CHECK:Function<foo>(2 params, 15 registers, 1 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
// CHECK-NEXT:    LoadParam         r3, 1
// CHECK-NEXT:    Dec               r2, r3
// CHECK-NEXT:    LoadConstZero     r3
// CHECK-NEXT:    LoadConstZero     r0
// CHECK-NEXT:    JmpFalse          L1, r2
// CHECK-NEXT:L2:
// CHECK-NEXT:    Add               r3, r3, r2
// CHECK-NEXT:    Dec               r2, r2
// CHECK-NEXT:    Mov               r0, r3
// CHECK-NEXT:    JmpTrue           L2, r2
// CHECK-NEXT:L1:
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    TryGetById        r1, r3, 1, "print"
// CHECK-NEXT:    LoadConstUndefined r3
// CHECK-NEXT:    LoadConstString   r2, "This\x0ais \u0435"...
// CHECK-NEXT:    Call3             r2, r1, r3, r2, r0
// CHECK-NEXT:    Ret               r3

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}pretty.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 0: line 10 col 1
// CHECK-NEXT:    bc 18: line 10 col 1
// CHECK-NEXT:  0x000a  function idx 1, starts at line 10 col 1
// CHECK-NEXT:    bc 3: line 12 col 12
// CHECK-NEXT:    bc 13: line 13 col 13
// CHECK-NEXT:    bc 28: line 14 col 5
// CHECK-NEXT:    bc 40: line 14 col 10
// CHECK-NEXT:  0x001a  end of debug source table
