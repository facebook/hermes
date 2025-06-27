/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-lir %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=LIR
// RUN: %hermesc -O -dump-bytecode %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=HBC

// Show that a sufficiently large switch with all string case labels generates
// a special IR and bytecode instruction, and that the bytecode file includes a
// string switch table.
(function() {
  function stringSwitch(s) {
    'noinline'
    switch (s) {
    case "l0":
      return 0;
    case "l1":
      return 1;
    case "l2":
      return 2;
    case "l3":
      return 3;
    case "l4":
      return 4;
    case "l5":
      return 5;
    case "l6":
      return 6;
    case "l7":
      return 7;
    case "l8":
      return 8;
    case "l9":
      return 9;
    }
    return 1000;
  }

  return stringSwitch("l1");
})();

// Auto-generated content below. Please do not modify manually.

// LIR:function global(): number
// LIR-NEXT:%BB0:
// LIR-NEXT:  %0 = LIRLoadConstInst (:string) "l1": string
// LIR-NEXT:  %1 = LIRLoadConstInst (:undefined) undefined: undefined
// LIR-NEXT:  %2 = CreateFunctionInst (:object) %1: undefined, empty: any, %stringSwitch(): functionCode
// LIR-NEXT:  %3 = HBCCallNInst (:number) %2: object, %stringSwitch(): functionCode, true: boolean, empty: any, undefined: undefined, %1: undefined, %0: string
// LIR-NEXT:       ReturnInst %3: number
// LIR-NEXT:function_end

// LIR:function stringSwitch(s: any): number [allCallsitesKnownInStrictMode]
// LIR-NEXT:%BB0:
// LIR-NEXT:  %0 = LoadParamInst (:any) %s: any
// LIR-NEXT:       StringSwitchImmInst %0: any, %BB1, 10: number, "l0": string, %BB2, "l1": string, %BB3, "l2": string, %BB4, "l3": string, %BB5, "l4": string, %BB6, "l5": string, %BB7, "l6": string, %BB8, "l7": string, %BB9, "l8": string, %BB10, "l9": string, %BB11
// LIR-NEXT:%BB1:
// LIR-NEXT:  %2 = LIRLoadConstInst (:number) 1000: number
// LIR-NEXT:       ReturnInst %2: number
// LIR-NEXT:%BB2:
// LIR-NEXT:  %4 = LIRLoadConstInst (:number) 0: number
// LIR-NEXT:       ReturnInst %4: number
// LIR-NEXT:%BB3:
// LIR-NEXT:  %6 = LIRLoadConstInst (:number) 1: number
// LIR-NEXT:       ReturnInst %6: number
// LIR-NEXT:%BB4:
// LIR-NEXT:  %8 = LIRLoadConstInst (:number) 2: number
// LIR-NEXT:       ReturnInst %8: number
// LIR-NEXT:%BB5:
// LIR-NEXT:  %10 = LIRLoadConstInst (:number) 3: number
// LIR-NEXT:        ReturnInst %10: number
// LIR-NEXT:%BB6:
// LIR-NEXT:  %12 = LIRLoadConstInst (:number) 4: number
// LIR-NEXT:        ReturnInst %12: number
// LIR-NEXT:%BB7:
// LIR-NEXT:  %14 = LIRLoadConstInst (:number) 5: number
// LIR-NEXT:        ReturnInst %14: number
// LIR-NEXT:%BB8:
// LIR-NEXT:  %16 = LIRLoadConstInst (:number) 6: number
// LIR-NEXT:        ReturnInst %16: number
// LIR-NEXT:%BB9:
// LIR-NEXT:  %18 = LIRLoadConstInst (:number) 7: number
// LIR-NEXT:        ReturnInst %18: number
// LIR-NEXT:%BB10:
// LIR-NEXT:  %20 = LIRLoadConstInst (:number) 8: number
// LIR-NEXT:        ReturnInst %20: number
// LIR-NEXT:%BB11:
// LIR-NEXT:  %22 = LIRLoadConstInst (:number) 9: number
// LIR-NEXT:        ReturnInst %22: number
// LIR-NEXT:function_end

// HBC:Bytecode File Information:
// HBC-NEXT:  Bytecode version number: {{.*}}
// HBC-NEXT:  Source hash: {{.*}}
// HBC-NEXT:  Function count: 2
// HBC-NEXT:  String count: 12
// HBC-NEXT:  BigInt count: 0
// HBC-NEXT:  String Kind Entry count: 1
// HBC-NEXT:  RegExp count: 0
// HBC-NEXT:  StringSwitchImm count: 1
// HBC-NEXT:  Segment ID: 0
// HBC-NEXT:  CommonJS module count: 0
// HBC-NEXT:  CommonJS module count (static): 0
// HBC-NEXT:  Function source count: 0
// HBC-NEXT:  Bytecode options:
// HBC-NEXT:    staticBuiltins: 0
// HBC-NEXT:    cjsModulesStaticallyResolved: 0

// HBC:Global String Table:
// HBC-NEXT:s0[ASCII, 0..5]: global
// HBC-NEXT:s1[ASCII, 5..6]: l0
// HBC-NEXT:s2[ASCII, 7..8]: l1
// HBC-NEXT:s3[ASCII, 9..10]: l2
// HBC-NEXT:s4[ASCII, 11..12]: l3
// HBC-NEXT:s5[ASCII, 13..14]: l4
// HBC-NEXT:s6[ASCII, 15..16]: l5
// HBC-NEXT:s7[ASCII, 17..18]: l6
// HBC-NEXT:s8[ASCII, 19..20]: l7
// HBC-NEXT:s9[ASCII, 21..22]: l8
// HBC-NEXT:s10[ASCII, 23..24]: l9
// HBC-NEXT:s11[ASCII, 25..36]: stringSwitch

// HBC:Function<global>(1 params, 13 registers, 1 numbers, 1 non-pointers):
// HBC-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// HBC-NEXT:    LoadConstString   r3, "l1"
// HBC-NEXT:    LoadConstUndefined r1
// HBC-NEXT:    CreateClosure     r2, r1, Function<stringSwitch>
// HBC-NEXT:    Call2             r0, r2, r1, r3
// HBC-NEXT:    Ret               r0

// HBC:Function<stringSwitch>(2 params, 2 registers, 1 numbers, 0 non-pointers):
// HBC-NEXT:    LoadParam         r1, 1
// HBC-NEXT:    StringSwitchImm   r1, 0, 75, L11, 10
// HBC-NEXT:L10:
// HBC-NEXT:    LoadConstUInt8    r0, 9
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L9:
// HBC-NEXT:    LoadConstUInt8    r0, 8
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L8:
// HBC-NEXT:    LoadConstUInt8    r0, 7
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L7:
// HBC-NEXT:    LoadConstUInt8    r0, 6
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L6:
// HBC-NEXT:    LoadConstUInt8    r0, 5
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L5:
// HBC-NEXT:    LoadConstUInt8    r0, 4
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L4:
// HBC-NEXT:    LoadConstUInt8    r0, 3
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L3:
// HBC-NEXT:    LoadConstUInt8    r0, 2
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L2:
// HBC-NEXT:    LoadConstUInt8    r0, 1
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L1:
// HBC-NEXT:    LoadConstZero     r0
// HBC-NEXT:    Ret               r0
// HBC-NEXT:L11:
// HBC-NEXT:    LoadConstInt      r0, 1000
// HBC-NEXT:    Ret               r0

// HBC: String Switch Tables:
// HBC-NEXT:  offset 75
// HBC-NEXT:   "l0" : L1
// HBC-NEXT:   "l1" : L2
// HBC-NEXT:   "l2" : L3
// HBC-NEXT:   "l3" : L4
// HBC-NEXT:   "l4" : L5
// HBC-NEXT:   "l5" : L6
// HBC-NEXT:   "l6" : L7
// HBC-NEXT:   "l7" : L8
// HBC-NEXT:   "l8" : L9
// HBC-NEXT:   "l9" : L10

// HBC:Debug filename table:
// HBC-NEXT:  0: {{.*}}switch-of-strings.js

// HBC:Debug file table:
// HBC-NEXT:  source table offset 0x0000: filename id 0

// HBC:Debug source table:
// HBC-NEXT:  0x0000  function idx 0, starts at line 14 col 1
// HBC-NEXT:    bc 11: line 42 col 22
// HBC-NEXT:  0x0009  end of debug source table
