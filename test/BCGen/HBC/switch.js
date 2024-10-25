/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheckOrRegen --match-full-lines %s

function f(x) {
  var str = "";
  switch (x) {
    case 0:
      return "regular";
    case 1:
    case 2:
      return "multicase";
    case 3:
      str +="fall";
    case 4:
      str +="through";
      return str;
    default:
      return "default";
  }
}

function regress1(w) {
  // This structure caused an IR verification failure.
  var v=0;
  while(true) {
    print(v);
    switch (w) {
      case 1:
        v=1;
      case 0:
        v=2;
    }
  }
}

function jump_table(x) {
    switch (x) {
        case 0:
	        return "foo"
        case 1:
	        return "bar"
        case 2:
	        return "baz"
        case 3:
            return "foo1"
        case 4:
            return "bar2"
        case 5:
            return "baz3"
        case 6:
            return "foo4"
        case 8:
            return "bar5"
        case 9:
            return "baz6"
        case 10:
            return "baz9"
    }
}

function string_switch(x) {
    switch (x) {
    case "a":
	return 1;
    case "b":
	return 2;
    case "c":
	return 3;
    }
}

function switch_uint32(x) {
  switch(x)
  {
    case 0x80000000:
    case 0x80000001:
    case 0x80000002:
    case 0x80000003:
    case 0x80000004:
    case 0x80000005:
    case 0x80000006:
    case 0x80000007:
    case 0x80000008:
    case 0x80000009:
      return 0;
  }
  return 1;
}

function switch_neg(x) {
  switch(x)
  {
    case -1:
    case -2:
    case -3:
    case -4:
    case -5:
    case -6:
    case -7:
    case -8:
    case -9:
      return 0;
  }
  return 1;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:                 DeclareGlobalVarInst "f": string
// CHECK-NEXT:                 DeclareGlobalVarInst "regress1": string
// CHECK-NEXT:                 DeclareGlobalVarInst "jump_table": string
// CHECK-NEXT:                 DeclareGlobalVarInst "string_switch": string
// CHECK-NEXT:                 DeclareGlobalVarInst "switch_uint32": string
// CHECK-NEXT:                 DeclareGlobalVarInst "switch_neg": string
// CHECK-NEXT:  {r2}      %7 = CreateFunctionInst (:object) {r0} %0: environment, %f(): functionCode
// CHECK-NEXT:  {r1}      %8 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %7: object, {r1} %8: object, "f": string
// CHECK-NEXT:  {r2}     %10 = CreateFunctionInst (:object) {r0} %0: environment, %regress1(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %10: object, {r1} %8: object, "regress1": string
// CHECK-NEXT:  {r2}     %12 = CreateFunctionInst (:object) {r0} %0: environment, %jump_table(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %12: object, {r1} %8: object, "jump_table": string
// CHECK-NEXT:  {r2}     %14 = CreateFunctionInst (:object) {r0} %0: environment, %string_switch(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %14: object, {r1} %8: object, "string_switch": string
// CHECK-NEXT:  {r2}     %16 = CreateFunctionInst (:object) {r0} %0: environment, %switch_uint32(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r2} %16: object, {r1} %8: object, "switch_uint32": string
// CHECK-NEXT:  {r0}     %18 = CreateFunctionInst (:object) {r0} %0: environment, %switch_neg(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {r0} %18: object, {r1} %8: object, "switch_neg": string
// CHECK-NEXT:  {np0}    %20 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %20: undefined
// CHECK-NEXT:function_end

// CHECK:function f(x: any): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:                 BranchInst %BB10
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {r0}      %2 = HBCLoadConstInst (:string) "regular": string
// CHECK-NEXT:                 ReturnInst {r0} %2: string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {r0}      %4 = HBCLoadConstInst (:string) "multicase": string
// CHECK-NEXT:                 ReturnInst {r0} %4: string
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {r0}      %6 = HBCLoadConstInst (:string) "fall": string
// CHECK-NEXT:  {r1}      %7 = MovInst (:string) {r0} %6: string
// CHECK-NEXT:                 BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  {r1}      %9 = PhiInst (:string) {r1} %7: string, %BB3, {r1} %17: string, %BB6
// CHECK-NEXT:  {r0}     %10 = HBCLoadConstInst (:string) "through": string
// CHECK-NEXT:  {r0}     %11 = HBCStringConcatInst (:string) {r1} %9: string, {r0} %10: string
// CHECK-NEXT:                 ReturnInst {r0} %11: string
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  {r0}     %13 = HBCLoadConstInst (:string) "default": string
// CHECK-NEXT:                 ReturnInst {r0} %13: string
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  {r1}     %15 = HBCLoadConstInst (:string) "": string
// CHECK-NEXT:  {n0}     %16 = HBCLoadConstInst (:number) 4: number
// CHECK-NEXT:  {r1}     %17 = MovInst (:string) {r1} %15: string
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %16: number, {r0} %0: any, %BB4, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  {n0}     %19 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %19: number, {r0} %0: any, %BB3, %BB6
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  {n0}     %21 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %21: number, {r0} %0: any, %BB2, %BB7
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  {n0}     %23 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %23: number, {r0} %0: any, %BB2, %BB8
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  {n0}     %25 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %25: number, {r0} %0: any, %BB1, %BB9
// CHECK-NEXT:function_end

// CHECK:function regress1(w: any): any [noReturn]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = LoadParamInst (:any) %w: any
// CHECK-NEXT:  {n3}      %1 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  {n2}      %2 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  {n1}      %3 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  {r0}      %4 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {np0}     %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  {n0}      %6 = MovInst (:number) {n3} %1: number
// CHECK-NEXT:                 BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {n0}      %8 = PhiInst (:number) {n0} %6: number, %BB0, {n0} %12: number, %BB2, {n0} %14: number, %BB3
// CHECK-NEXT:  {r2}      %9 = TryLoadGlobalPropertyInst (:any) {r0} %4: object, "print": string
// CHECK-NEXT:  {r2}     %10 = HBCCallNInst (:any) {r2} %9: any, empty: any, false: boolean, empty: any, undefined: undefined, {np0} %5: undefined, {n0} %8: number
// CHECK-NEXT:                 BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {n0}     %12 = MovInst (:number) {n2} %2: number
// CHECK-NEXT:                 BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {n0}     %14 = MovInst (:number) {n0} %8: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n3} %1: number, {r1} %0: any, %BB2, %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n1} %3: number, {r1} %0: any, %BB2, %BB3
// CHECK-NEXT:function_end

// CHECK:function jump_table(x: any): undefined|string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:                 SwitchImmInst {r0} %0: any, %BB1, 0: number, 11: number, 0: number, %BB2, 1: number, %BB3, 2: number, %BB4, 3: number, %BB5, 4: number, %BB6, 5: number, %BB7, 6: number, %BB8, 8: number, %BB9, 9: number, %BB10, 10: number, %BB11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {np0}     %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %2: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {r0}      %4 = HBCLoadConstInst (:string) "foo": string
// CHECK-NEXT:                 ReturnInst {r0} %4: string
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {r0}      %6 = HBCLoadConstInst (:string) "bar": string
// CHECK-NEXT:                 ReturnInst {r0} %6: string
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  {r0}      %8 = HBCLoadConstInst (:string) "baz": string
// CHECK-NEXT:                 ReturnInst {r0} %8: string
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  {r0}     %10 = HBCLoadConstInst (:string) "foo1": string
// CHECK-NEXT:                 ReturnInst {r0} %10: string
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  {r0}     %12 = HBCLoadConstInst (:string) "bar2": string
// CHECK-NEXT:                 ReturnInst {r0} %12: string
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  {r0}     %14 = HBCLoadConstInst (:string) "baz3": string
// CHECK-NEXT:                 ReturnInst {r0} %14: string
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  {r0}     %16 = HBCLoadConstInst (:string) "foo4": string
// CHECK-NEXT:                 ReturnInst {r0} %16: string
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  {r0}     %18 = HBCLoadConstInst (:string) "bar5": string
// CHECK-NEXT:                 ReturnInst {r0} %18: string
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  {r0}     %20 = HBCLoadConstInst (:string) "baz6": string
// CHECK-NEXT:                 ReturnInst {r0} %20: string
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  {r0}     %22 = HBCLoadConstInst (:string) "baz9": string
// CHECK-NEXT:                 ReturnInst {r0} %22: string
// CHECK-NEXT:function_end

// CHECK:function string_switch(x: any): undefined|number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r1}      %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:                 BranchInst %BB7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {np0}     %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %2: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {n0}      %4 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:                 ReturnInst {n0} %4: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {n0}      %6 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:                 ReturnInst {n0} %6: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  {n0}      %8 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:                 ReturnInst {n0} %8: number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  {r0}     %10 = HBCLoadConstInst (:string) "c": string
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {r0} %10: string, {r1} %0: any, %BB4, %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  {r0}     %12 = HBCLoadConstInst (:string) "b": string
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {r0} %12: string, {r1} %0: any, %BB3, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  {r0}     %14 = HBCLoadConstInst (:string) "a": string
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {r0} %14: string, {r1} %0: any, %BB2, %BB6
// CHECK-NEXT:function_end

// CHECK:function switch_uint32(x: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:                 SwitchImmInst {r0} %0: any, %BB1, 2147483648: number, 10: number, 2147483648: number, %BB2, 2147483649: number, %BB2, 2147483650: number, %BB2, 2147483651: number, %BB2, 2147483652: number, %BB2, 2147483653: number, %BB2, 2147483654: number, %BB2, 2147483655: number, %BB2, 2147483656: number, %BB2, 2147483657: number, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {n0}      %2 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:                 ReturnInst {n0} %2: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {n0}      %4 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:                 ReturnInst {n0} %4: number
// CHECK-NEXT:function_end

// CHECK:function switch_neg(x: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {r0}      %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:                 BranchInst %BB11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {n0}      %2 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:                 ReturnInst {n0} %2: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  {n0}      %4 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:                 ReturnInst {n0} %4: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  {n0}      %6 = HBCLoadConstInst (:number) -9: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %6: number, {r0} %0: any, %BB2, %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  {n0}      %8 = HBCLoadConstInst (:number) -8: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %8: number, {r0} %0: any, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  {n0}     %10 = HBCLoadConstInst (:number) -7: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %10: number, {r0} %0: any, %BB2, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  {n0}     %12 = HBCLoadConstInst (:number) -6: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %12: number, {r0} %0: any, %BB2, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  {n0}     %14 = HBCLoadConstInst (:number) -5: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %14: number, {r0} %0: any, %BB2, %BB6
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  {n0}     %16 = HBCLoadConstInst (:number) -4: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %16: number, {r0} %0: any, %BB2, %BB7
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  {n0}     %18 = HBCLoadConstInst (:number) -3: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %18: number, {r0} %0: any, %BB2, %BB8
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  {n0}     %20 = HBCLoadConstInst (:number) -2: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %20: number, {r0} %0: any, %BB2, %BB9
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  {n0}     %22 = HBCLoadConstInst (:number) -1: number
// CHECK-NEXT:                 CmpBrStrictlyEqualInst {n0} %22: number, {r0} %0: any, %BB2, %BB10
// CHECK-NEXT:function_end
