/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-lir %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=LIR
// RUN: %hermesc -O -dump-bytecode %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=HBC

// This test ensure that the string tables are created correctly when
// there are multiple switches in a function, including by a UInt
// jump-table switch and several string switches.
function stringSwitchSeveralSwitches(b, s, i) {
  'noinline'
  var res = 0;
  switch (i) {
  case 1:
    res = 10000;
    break;
  case 2:
    res = 20000;
    break;
  case 3:
    res = 30000;
    break;
  case 4:
    res = 50000;
    break;
  case 5:
    res = 50000;
    break;
  case 6:
    res = 60000;
    break;
  case 7:
    res = 70000;
    break;
  case 8:
    res = 80000;
    break;
  case 9:
    res = 90000;
    break;
  case 10:
    res = 100000;
    break;
  }
  if (b) {
    switch (s) {
      case "l0":
        res += 10;
        break;
      case "l1":
        res += 11;
        break;
      case "l2":
        res += 12;
        break;
      case "l3":
        res += 13;
        break;
      case "l4":
        res += 14;
        break;
      case "l5":
        res += 15;
        break;
      case "l6":
        res += 16;
        break;
      case "l7":
        res += 17;
        break;
      case "l8":
        res += 18;
        break;
      case "l9":
        res += 19;
        break;
    }
  } else {
    switch (s) {
      case "l1000":
        res += 1000;
        break;
      case "l1100":
        res += 1100;
        break;
      case "l1200":
        res += 1200;
        break;
      case "l1300":
        res += 1300;
        break;
      case "l1400":
        res += 1400;
        break;
      case "l1500":
        res += 1500;
        break;
      case "l1600":
        res += 1600;
        break;
      case "l1700":
        res += 1700;
        break;
      case "l1800":
        res += 1800;
        break;
      case "l1900":
        res += 1900;
        break;
    }
  }
  return res;
}

// Auto-generated content below. Please do not modify manually.

// LIR:function global(): undefined
// LIR-NEXT:%BB0:
// LIR-NEXT:       DeclareGlobalVarInst "stringSwitchSeveralSwitches": string
// LIR-NEXT:  %1 = LIRGetGlobalObjectInst (:object)
// LIR-NEXT:  %2 = LIRLoadConstInst (:undefined) undefined: undefined
// LIR-NEXT:  %3 = CreateFunctionInst (:object) %2: undefined, empty: any, %stringSwitchSeveralSwitches(): functionCode
// LIR-NEXT:       StorePropertyLooseInst %3: object, %1: object, "stringSwitchSeveralSwitches": string
// LIR-NEXT:       ReturnInst %2: undefined
// LIR-NEXT:function_end

// LIR:function stringSwitchSeveralSwitches(b: any, s: any, i: any): number
// LIR-NEXT:%BB0:
// LIR-NEXT:  %0 = LIRLoadConstInst (:number) 0: number
// LIR-NEXT:  %1 = LoadParamInst (:any) %i: any
// LIR-NEXT:       UIntSwitchImmInst %1: any, %BB1, 10: number, 1: number, 1: number, %BB2, 2: number, %BB3, 3: number, %BB4, 4: number, %BB5, 5: number, %BB6, 6: number, %BB7, 7: number, %BB8, 8: number, %BB9, 9: number, %BB10, 10: number, %BB11
// LIR-NEXT:%BB1:
// LIR-NEXT:  %3 = PhiInst (:number) %7: number, %BB2, %9: number, %BB3, %11: number, %BB4, %13: number, %BB5, %15: number, %BB6, %17: number, %BB7, %19: number, %BB8, %21: number, %BB9, %23: number, %BB10, %25: number, %BB11, %0: number, %BB0
// LIR-NEXT:  %4 = LoadParamInst (:any) %b: any
// LIR-NEXT:  %5 = LoadParamInst (:any) %s: any
// LIR-NEXT:       CondBranchInst %4: any, %BB12, %BB13
// LIR-NEXT:%BB2:
// LIR-NEXT:  %7 = LIRLoadConstInst (:number) 10000: number
// LIR-NEXT:       BranchInst %BB1
// LIR-NEXT:%BB3:
// LIR-NEXT:  %9 = LIRLoadConstInst (:number) 20000: number
// LIR-NEXT:        BranchInst %BB1
// LIR-NEXT:%BB4:
// LIR-NEXT:  %11 = LIRLoadConstInst (:number) 30000: number
// LIR-NEXT:        BranchInst %BB1
// LIR-NEXT:%BB5:
// LIR-NEXT:  %13 = LIRLoadConstInst (:number) 50000: number
// LIR-NEXT:        BranchInst %BB1
// LIR-NEXT:%BB6:
// LIR-NEXT:  %15 = LIRLoadConstInst (:number) 50000: number
// LIR-NEXT:        BranchInst %BB1
// LIR-NEXT:%BB7:
// LIR-NEXT:  %17 = LIRLoadConstInst (:number) 60000: number
// LIR-NEXT:        BranchInst %BB1
// LIR-NEXT:%BB8:
// LIR-NEXT:  %19 = LIRLoadConstInst (:number) 70000: number
// LIR-NEXT:        BranchInst %BB1
// LIR-NEXT:%BB9:
// LIR-NEXT:  %21 = LIRLoadConstInst (:number) 80000: number
// LIR-NEXT:        BranchInst %BB1
// LIR-NEXT:%BB10:
// LIR-NEXT:  %23 = LIRLoadConstInst (:number) 90000: number
// LIR-NEXT:        BranchInst %BB1
// LIR-NEXT:%BB11:
// LIR-NEXT:  %25 = LIRLoadConstInst (:number) 100000: number
// LIR-NEXT:        BranchInst %BB1
// LIR-NEXT:%BB12:
// LIR-NEXT:        StringSwitchImmInst %5: any, %BB14, 10: number, "l0": string, %BB15, "l1": string, %BB16, "l2": string, %BB17, "l3": string, %BB18, "l4": string, %BB19, "l5": string, %BB20, "l6": string, %BB21, "l7": string, %BB22, "l8": string, %BB23, "l9": string, %BB24
// LIR-NEXT:%BB13:
// LIR-NEXT:        StringSwitchImmInst %5: any, %BB14, 10: number, "l1000": string, %BB25, "l1100": string, %BB26, "l1200": string, %BB27, "l1300": string, %BB28, "l1400": string, %BB29, "l1500": string, %BB30, "l1600": string, %BB31, "l1700": string, %BB32, "l1800": string, %BB33, "l1900": string, %BB34
// LIR-NEXT:%BB14:
// LIR-NEXT:  %29 = PhiInst (:number) %62: number, %BB25, %65: number, %BB26, %3: number, %BB12, %59: number, %BB24, %56: number, %BB23, %53: number, %BB22, %50: number, %BB21, %47: number, %BB20, %44: number, %BB19, %41: number, %BB18, %38: number, %BB17, %35: number, %BB16, %32: number, %BB15, %3: number, %BB13, %89: number, %BB34, %86: number, %BB33, %83: number, %BB32, %80: number, %BB31, %77: number, %BB30, %74: number, %BB29, %71: number, %BB28, %68: number, %BB27
// LIR-NEXT:        ReturnInst %29: number
// LIR-NEXT:%BB15:
// LIR-NEXT:  %31 = LIRLoadConstInst (:number) 10: number
// LIR-NEXT:  %32 = FAddInst (:number) %3: number, %31: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB16:
// LIR-NEXT:  %34 = LIRLoadConstInst (:number) 11: number
// LIR-NEXT:  %35 = FAddInst (:number) %3: number, %34: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB17:
// LIR-NEXT:  %37 = LIRLoadConstInst (:number) 12: number
// LIR-NEXT:  %38 = FAddInst (:number) %3: number, %37: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB18:
// LIR-NEXT:  %40 = LIRLoadConstInst (:number) 13: number
// LIR-NEXT:  %41 = FAddInst (:number) %3: number, %40: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB19:
// LIR-NEXT:  %43 = LIRLoadConstInst (:number) 14: number
// LIR-NEXT:  %44 = FAddInst (:number) %3: number, %43: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB20:
// LIR-NEXT:  %46 = LIRLoadConstInst (:number) 15: number
// LIR-NEXT:  %47 = FAddInst (:number) %3: number, %46: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB21:
// LIR-NEXT:  %49 = LIRLoadConstInst (:number) 16: number
// LIR-NEXT:  %50 = FAddInst (:number) %3: number, %49: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB22:
// LIR-NEXT:  %52 = LIRLoadConstInst (:number) 17: number
// LIR-NEXT:  %53 = FAddInst (:number) %3: number, %52: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB23:
// LIR-NEXT:  %55 = LIRLoadConstInst (:number) 18: number
// LIR-NEXT:  %56 = FAddInst (:number) %3: number, %55: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB24:
// LIR-NEXT:  %58 = LIRLoadConstInst (:number) 19: number
// LIR-NEXT:  %59 = FAddInst (:number) %3: number, %58: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB25:
// LIR-NEXT:  %61 = LIRLoadConstInst (:number) 1000: number
// LIR-NEXT:  %62 = FAddInst (:number) %3: number, %61: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB26:
// LIR-NEXT:  %64 = LIRLoadConstInst (:number) 1100: number
// LIR-NEXT:  %65 = FAddInst (:number) %3: number, %64: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB27:
// LIR-NEXT:  %67 = LIRLoadConstInst (:number) 1200: number
// LIR-NEXT:  %68 = FAddInst (:number) %3: number, %67: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB28:
// LIR-NEXT:  %70 = LIRLoadConstInst (:number) 1300: number
// LIR-NEXT:  %71 = FAddInst (:number) %3: number, %70: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB29:
// LIR-NEXT:  %73 = LIRLoadConstInst (:number) 1400: number
// LIR-NEXT:  %74 = FAddInst (:number) %3: number, %73: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB30:
// LIR-NEXT:  %76 = LIRLoadConstInst (:number) 1500: number
// LIR-NEXT:  %77 = FAddInst (:number) %3: number, %76: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB31:
// LIR-NEXT:  %79 = LIRLoadConstInst (:number) 1600: number
// LIR-NEXT:  %80 = FAddInst (:number) %3: number, %79: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB32:
// LIR-NEXT:  %82 = LIRLoadConstInst (:number) 1700: number
// LIR-NEXT:  %83 = FAddInst (:number) %3: number, %82: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB33:
// LIR-NEXT:  %85 = LIRLoadConstInst (:number) 1800: number
// LIR-NEXT:  %86 = FAddInst (:number) %3: number, %85: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:%BB34:
// LIR-NEXT:  %88 = LIRLoadConstInst (:number) 1900: number
// LIR-NEXT:  %89 = FAddInst (:number) %3: number, %88: number
// LIR-NEXT:        BranchInst %BB14
// LIR-NEXT:function_end

// HBC:Bytecode File Information:
// HBC-NEXT:  Bytecode version number: {{.*}}
// HBC-NEXT:  Source hash: {{.*}}
// HBC-NEXT:  Function count: 2
// HBC-NEXT:  String count: 22
// HBC-NEXT:  BigInt count: 0
// HBC-NEXT:  String Kind Entry count: 2
// HBC-NEXT:  RegExp count: 0
// HBC-NEXT:  StringSwitchImm count: 2
// HBC-NEXT:  Key buffer size (bytes): 0
// HBC-NEXT:  Value buffer size (bytes): 0
// HBC-NEXT:  Shape table count: 0
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
// HBC-NEXT:s3[ASCII, 7..11]: l1000
// HBC-NEXT:s4[ASCII, 12..16]: l1100
// HBC-NEXT:s5[ASCII, 17..21]: l1200
// HBC-NEXT:s6[ASCII, 22..26]: l1300
// HBC-NEXT:s7[ASCII, 27..31]: l1400
// HBC-NEXT:s8[ASCII, 32..36]: l1500
// HBC-NEXT:s9[ASCII, 37..41]: l1600
// HBC-NEXT:s10[ASCII, 42..46]: l1700
// HBC-NEXT:s11[ASCII, 47..51]: l1800
// HBC-NEXT:s12[ASCII, 52..56]: l1900
// HBC-NEXT:s13[ASCII, 57..58]: l2
// HBC-NEXT:s14[ASCII, 59..60]: l3
// HBC-NEXT:s15[ASCII, 61..62]: l4
// HBC-NEXT:s16[ASCII, 63..64]: l5
// HBC-NEXT:s17[ASCII, 65..66]: l6
// HBC-NEXT:s18[ASCII, 67..68]: l7
// HBC-NEXT:s19[ASCII, 69..70]: l8
// HBC-NEXT:s20[ASCII, 71..72]: l9
// HBC-NEXT:i21[ASCII, 73..99] #88138FB1: stringSwitchSeveralSwitches

// HBC:Function<global>(1 params, 3 registers, 0 numbers, 1 non-pointers):
// HBC-NEXT:Offset in debug table: source 0x0000
// HBC-NEXT:    DeclareGlobalVar  "stringSwitchSever"...
// HBC-NEXT:    GetGlobalObject   r2
// HBC-NEXT:    LoadConstUndefined r0
// HBC-NEXT:    CreateClosure     r1, r0, Function<stringSwitchSeveralSwitches>
// HBC-NEXT:    PutByIdLoose      r2, r1, 0, "stringSwitchSever"...
// HBC-NEXT:    Ret               r0

// HBC:Function<stringSwitchSeveralSwitches>(4 params, 5 registers, 3 numbers, 0 non-pointers):
// HBC-NEXT:    LoadParam         r3, 3
// HBC-NEXT:    LoadConstZero     r0
// HBC-NEXT:    UIntSwitchImm     r3, 384, L11, 1, 10
// HBC-NEXT:L10:
// HBC-NEXT:    LoadConstInt      r0, 100000
// HBC-NEXT:    Jmp               L11
// HBC-NEXT:L9:
// HBC-NEXT:    LoadConstInt      r0, 90000
// HBC-NEXT:    Jmp               L11
// HBC-NEXT:L8:
// HBC-NEXT:    LoadConstInt      r0, 80000
// HBC-NEXT:    Jmp               L11
// HBC-NEXT:L7:
// HBC-NEXT:    LoadConstInt      r0, 70000
// HBC-NEXT:    Jmp               L11
// HBC-NEXT:L6:
// HBC-NEXT:    LoadConstInt      r0, 60000
// HBC-NEXT:    Jmp               L11
// HBC-NEXT:L5:
// HBC-NEXT:    LoadConstInt      r0, 50000
// HBC-NEXT:    Jmp               L11
// HBC-NEXT:L4:
// HBC-NEXT:    LoadConstInt      r0, 50000
// HBC-NEXT:    Jmp               L11
// HBC-NEXT:L3:
// HBC-NEXT:    LoadConstInt      r0, 30000
// HBC-NEXT:    Jmp               L11
// HBC-NEXT:L2:
// HBC-NEXT:    LoadConstInt      r0, 20000
// HBC-NEXT:    Jmp               L11
// HBC-NEXT:L1:
// HBC-NEXT:    LoadConstInt      r0, 10000
// HBC-NEXT:L11:
// HBC-NEXT:    LoadParam         r4, 1
// HBC-NEXT:    LoadParam         r3, 2
// HBC-NEXT:    JmpTrueLong       L12, r4
// HBC-NEXT:    Mov               r1, r0
// HBC-NEXT:    StringSwitchImm   r3, 0, 313, L23, 10
// HBC-NEXT:L22:
// HBC-NEXT:    LoadConstInt      r2, 1900
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    JmpLong           L23
// HBC-NEXT:L21:
// HBC-NEXT:    LoadConstInt      r2, 1800
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    JmpLong           L23
// HBC-NEXT:L20:
// HBC-NEXT:    LoadConstInt      r2, 1700
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    JmpLong           L23
// HBC-NEXT:L19:
// HBC-NEXT:    LoadConstInt      r2, 1600
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    JmpLong           L23
// HBC-NEXT:L18:
// HBC-NEXT:    LoadConstInt      r2, 1500
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    JmpLong           L23
// HBC-NEXT:L17:
// HBC-NEXT:    LoadConstInt      r2, 1400
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    JmpLong           L23
// HBC-NEXT:L16:
// HBC-NEXT:    LoadConstInt      r2, 1300
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    JmpLong           L23
// HBC-NEXT:L15:
// HBC-NEXT:    LoadConstInt      r2, 1200
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    JmpLong           L23
// HBC-NEXT:L14:
// HBC-NEXT:    LoadConstInt      r2, 1100
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L13:
// HBC-NEXT:    LoadConstInt      r2, 1000
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L12:
// HBC-NEXT:    Mov               r1, r0
// HBC-NEXT:    StringSwitchImm   r3, 1, 228, L23, 10
// HBC-NEXT:L33:
// HBC-NEXT:    LoadConstUInt8    r2, 19
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L32:
// HBC-NEXT:    LoadConstUInt8    r2, 18
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L31:
// HBC-NEXT:    LoadConstUInt8    r2, 17
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L30:
// HBC-NEXT:    LoadConstUInt8    r2, 16
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L29:
// HBC-NEXT:    LoadConstUInt8    r2, 15
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L28:
// HBC-NEXT:    LoadConstUInt8    r2, 14
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L27:
// HBC-NEXT:    LoadConstUInt8    r2, 13
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L26:
// HBC-NEXT:    LoadConstUInt8    r2, 12
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L25:
// HBC-NEXT:    LoadConstUInt8    r2, 11
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:    Jmp               L23
// HBC-NEXT:L24:
// HBC-NEXT:    LoadConstUInt8    r2, 10
// HBC-NEXT:    AddN              r1, r0, r2
// HBC-NEXT:L23:
// HBC-NEXT:    Ret               r1

// HBC: Jump Tables:
// HBC-NEXT:  offset 384
// HBC-NEXT:   1 : L1
// HBC-NEXT:   2 : L2
// HBC-NEXT:   3 : L3
// HBC-NEXT:   4 : L4
// HBC-NEXT:   5 : L5
// HBC-NEXT:   6 : L6
// HBC-NEXT:   7 : L7
// HBC-NEXT:   8 : L8
// HBC-NEXT:   9 : L9
// HBC-NEXT:   10 : L10

// HBC: String Switch Tables:
// HBC-NEXT:  offset 313
// HBC-NEXT:   "l1000" : L13
// HBC-NEXT:   "l1100" : L14
// HBC-NEXT:   "l1200" : L15
// HBC-NEXT:   "l1300" : L16
// HBC-NEXT:   "l1400" : L17
// HBC-NEXT:   "l1500" : L18
// HBC-NEXT:   "l1600" : L19
// HBC-NEXT:   "l1700" : L20
// HBC-NEXT:   "l1800" : L21
// HBC-NEXT:   "l1900" : L22
// HBC-NEXT:  offset 228
// HBC-NEXT:   "l0" : L24
// HBC-NEXT:   "l1" : L25
// HBC-NEXT:   "l2" : L26
// HBC-NEXT:   "l3" : L27
// HBC-NEXT:   "l4" : L28
// HBC-NEXT:   "l5" : L29
// HBC-NEXT:   "l6" : L30
// HBC-NEXT:   "l7" : L31
// HBC-NEXT:   "l8" : L32
// HBC-NEXT:   "l9" : L33

// HBC:Debug filename table:
// HBC-NEXT:  0: {{.*}}switch-of-strings-multiple-switches.js

// HBC:Debug file table:
// HBC-NEXT:  source table offset 0x0000: filename id 0

// HBC:Debug source table:
// HBC-NEXT:  0x0000  function idx 0, starts at line 14 col 1
// HBC-NEXT:    bc 0: line 14 col 1
// HBC-NEXT:    bc 14: line 14 col 1
// HBC-NEXT:  0x000b  end of debug source table
