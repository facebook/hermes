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
// CHECK-NEXT:  $Reg0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "f": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "regress1": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "jump_table": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "string_switch": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "switch_uint32": string
// CHECK-NEXT:  $Reg1 = DeclareGlobalVarInst "switch_neg": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg0, %f(): functionCode
// CHECK-NEXT:  $Reg1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "f": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg0, %regress1(): functionCode
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "regress1": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg0, %jump_table(): functionCode
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "jump_table": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg0, %string_switch(): functionCode
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "string_switch": string
// CHECK-NEXT:  $Reg2 = CreateFunctionInst (:object) $Reg0, %switch_uint32(): functionCode
// CHECK-NEXT:  $Reg2 = StorePropertyLooseInst $Reg2, $Reg1, "switch_uint32": string
// CHECK-NEXT:  $Reg0 = CreateFunctionInst (:object) $Reg0, %switch_neg(): functionCode
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg0, $Reg1, "switch_neg": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function f(x: any): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg0 = BranchInst %BB10
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "regular": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "multicase": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "fall": string
// CHECK-NEXT:  $Reg1 = MovInst (:string) $Reg0
// CHECK-NEXT:  $Reg0 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg1 = PhiInst (:string) $Reg1, %BB3, $Reg1, %BB6
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "through": string
// CHECK-NEXT:  $Reg0 = HBCStringConcatInst (:string) $Reg1, $Reg0
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "default": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:string) "": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 4: number
// CHECK-NEXT:  $Reg1 = MovInst (:string) $Reg1
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg2, %BB4, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg2, %BB3, %BB6
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg2, %BB2, %BB7
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg2, %BB2, %BB8
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg2, %BB1, %BB9
// CHECK-NEXT:function_end

// CHECK:function regress1(w: any): any [noReturn]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg6 = LoadParamInst (:any) %w: any
// CHECK-NEXT:  $Reg5 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  $Reg3 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = MovInst (:number) $Reg5
// CHECK-NEXT:  $Reg7 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 = PhiInst (:number) $Reg0, %BB0, $Reg0, %BB2, $Reg0, %BB3
// CHECK-NEXT:  $Reg7 = TryLoadGlobalPropertyInst (:any) $Reg2, "print": string
// CHECK-NEXT:  $Reg7 = HBCCallNInst (:any) $Reg7, empty: any, false: boolean, empty: any, undefined: undefined, $Reg1, $Reg0
// CHECK-NEXT:  $Reg7 = BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = MovInst (:number) $Reg4
// CHECK-NEXT:  $Reg0 = BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 = MovInst (:number) $Reg0
// CHECK-NEXT:  $Reg7 = CmpBrStrictlyEqualInst $Reg5, $Reg6, %BB2, %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg7 = CmpBrStrictlyEqualInst $Reg3, $Reg6, %BB2, %BB3
// CHECK-NEXT:function_end

// CHECK:function jump_table(x: any): undefined|string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg0 = SwitchImmInst $Reg0, %BB1, 0: number, 11: number, 0: number, %BB2, 1: number, %BB3, 2: number, %BB4, 3: number, %BB5, 4: number, %BB6, 5: number, %BB7, 6: number, %BB8, 8: number, %BB9, 9: number, %BB10, 10: number, %BB11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "foo": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "bar": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "baz": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "foo1": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "bar2": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "baz3": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "foo4": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "bar5": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "baz6": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "baz9": string
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function string_switch(x: any): undefined|number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg0 = BranchInst %BB7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "c": string
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB4, %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "b": string
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB3, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:string) "a": string
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB6
// CHECK-NEXT:function_end

// CHECK:function switch_uint32(x: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg0 = SwitchImmInst $Reg0, %BB1, 2147483648: number, 10: number, 2147483648: number, %BB2, 2147483649: number, %BB2, 2147483650: number, %BB2, 2147483651: number, %BB2, 2147483652: number, %BB2, 2147483653: number, %BB2, 2147483654: number, %BB2, 2147483655: number, %BB2, 2147483656: number, %BB2, 2147483657: number, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function switch_neg(x: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg0 = BranchInst %BB11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) -9: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) -8: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) -7: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) -6: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) -5: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB6
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) -4: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB7
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) -3: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB8
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) -2: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB9
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:number) -1: number
// CHECK-NEXT:  $Reg0 = CmpBrStrictlyEqualInst $Reg0, $Reg1, %BB2, %BB10
// CHECK-NEXT:function_end
