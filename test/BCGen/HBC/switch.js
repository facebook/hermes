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

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [f, regress1, jump_table, string_switch, switch_uint32]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...11) 	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg2 @1 [2...4) 	%1 = HBCCreateFunctionInst %f#0#1()#2 : string, %0
// CHECK-NEXT:  $Reg1 @2 [3...12) 	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg2 @3 [empty]	%3 = StorePropertyInst %1 : closure, %2 : object, "f" : string
// CHECK-NEXT:  $Reg2 @4 [5...6) 	%4 = HBCCreateFunctionInst %regress1#0#1()#3 : undefined, %0
// CHECK-NEXT:  $Reg2 @5 [empty]	%5 = StorePropertyInst %4 : closure, %2 : object, "regress1" : string
// CHECK-NEXT:  $Reg2 @6 [7...8) 	%6 = HBCCreateFunctionInst %jump_table#0#1()#4 : undefined|string, %0
// CHECK-NEXT:  $Reg2 @7 [empty]	%7 = StorePropertyInst %6 : closure, %2 : object, "jump_table" : string
// CHECK-NEXT:  $Reg2 @8 [9...10) 	%8 = HBCCreateFunctionInst %string_switch#0#1()#5 : undefined|number, %0
// CHECK-NEXT:  $Reg2 @9 [empty]	%9 = StorePropertyInst %8 : closure, %2 : object, "string_switch" : string
// CHECK-NEXT:  $Reg0 @10 [11...12) 	%10 = HBCCreateFunctionInst %switch_uint32#0#1()#6 : number, %0
// CHECK-NEXT:  $Reg0 @11 [empty]	%11 = StorePropertyInst %10 : closure, %2 : object, "switch_uint32" : string
// CHECK-NEXT:  $Reg0 @12 [13...14) 	%12 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @13 [empty]	%13 = ReturnInst %12 : undefined
// CHECK-NEXT:function_end

// CHECK:function f#0#1(x)#2 : string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg3 @0 [1...15) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg0 @1 [2...18) 	%1 = HBCLoadConstInst "fall" : string
// CHECK-NEXT:  $Reg1 @2 [3...14) 	%2 = HBCLoadConstInst "" : string
// CHECK-NEXT:  $Reg2 @3 [empty]	%3 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @25 [26...27) 	%4 = HBCLoadConstInst "regular" : string
// CHECK-NEXT:  $Reg0 @26 [empty]	%5 = ReturnInst %4 : string
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 @23 [24...25) 	%6 = HBCLoadConstInst "multicase" : string
// CHECK-NEXT:  $Reg0 @24 [empty]	%7 = ReturnInst %6 : string
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg1 @17 [18...20) 	%8 = MovInst %1 : string
// CHECK-NEXT:  $Reg0 @18 [empty]	%9 = BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg1 @19 [3...22) 	%10 = PhiInst %8 : string, %BB4, %17 : string, %BB6
// CHECK-NEXT:  $Reg0 @20 [21...22) 	%11 = HBCLoadConstInst "through" : string
// CHECK-NEXT:  $Reg0 @21 [22...23) 	%12 = BinaryOperatorInst '+', %10 : string, %11 : string
// CHECK-NEXT:  $Reg0 @22 [empty]	%13 = ReturnInst %12 : string
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  $Reg2 @15 [16...17) 	%14 = HBCLoadConstInst "default" : string
// CHECK-NEXT:  $Reg2 @16 [empty]	%15 = ReturnInst %14 : string
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  $Reg2 @12 [13...15) 	%16 = HBCLoadConstInst 4 : number
// CHECK-NEXT:  $Reg1 @13 [14...20) 	%17 = MovInst %2 : string
// CHECK-NEXT:  $Reg2 @14 [empty]	%18 = CompareBranchInst '===', %16 : number, %0, %BB5, %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  $Reg2 @10 [11...12) 	%19 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg2 @11 [empty]	%20 = CompareBranchInst '===', %19 : number, %0, %BB4, %BB6
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  $Reg2 @8 [9...10) 	%21 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg2 @9 [empty]	%22 = CompareBranchInst '===', %21 : number, %0, %BB3, %BB8
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  $Reg2 @6 [7...8) 	%23 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg2 @7 [empty]	%24 = CompareBranchInst '===', %23 : number, %0, %BB3, %BB9
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg2 @4 [5...6) 	%25 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg2 @5 [empty]	%26 = CompareBranchInst '===', %25 : number, %0, %BB2, %BB10
// CHECK-NEXT:function_end

// CHECK:function regress1#0#1(w)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg6 @0 [1...17) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg5 @1 [2...17) 	%1 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg4 @2 [3...17) 	%2 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg3 @3 [4...17) 	%3 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg2 @4 [5...17) 	%4 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg1 @5 [6...17) 	%5 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg0 @6 [7...9) 	%6 = MovInst %1 : number
// CHECK-NEXT:  $Reg7 @7 [empty]	%7 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 @8 [7...15) [16...17) 	%8 = PhiInst %6 : number, %BB0, %12 : number, %BB2, %14 : number, %BB3
// CHECK-NEXT:  $Reg7 @9 [10...11) 	%9 = TryLoadGlobalPropertyInst %3 : object, "print" : string
// CHECK-NEXT:  $Reg7 @10 [empty]	%10 = HBCCallNInst %9, %4 : undefined, %8 : number
// CHECK-NEXT:  $Reg7 @11 [empty]	%11 = BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @15 [16...17) 	%12 = MovInst %2 : number
// CHECK-NEXT:  $Reg0 @16 [empty]	%13 = BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 @13 [14...15) 	%14 = MovInst %8 : number
// CHECK-NEXT:  $Reg7 @14 [empty]	%15 = CompareBranchInst '===', %1 : number, %0, %BB2, %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg7 @12 [empty]	%16 = CompareBranchInst '===', %5 : number, %0, %BB2, %BB3
// CHECK-NEXT:function_end

// CHECK:function jump_table#0#1(x)#2 : undefined|string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg0 @1 [empty]	%1 = SwitchImmInst %0, %BB1, 0 : number, 11 : number, 0 : number, %BB2, 1 : number, %BB3, 2 : number, %BB4, 3 : number, %BB5, 4 : number, %BB6, 5 : number, %BB7, 6 : number, %BB8, 8 : number, %BB9, 9 : number, %BB10, 10 : number, %BB11
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 @22 [23...24) 	%2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @23 [empty]	%3 = ReturnInst %2 : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @20 [21...22) 	%4 = HBCLoadConstInst "foo" : string
// CHECK-NEXT:  $Reg0 @21 [empty]	%5 = ReturnInst %4 : string
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 @18 [19...20) 	%6 = HBCLoadConstInst "bar" : string
// CHECK-NEXT:  $Reg0 @19 [empty]	%7 = ReturnInst %6 : string
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg0 @16 [17...18) 	%8 = HBCLoadConstInst "baz" : string
// CHECK-NEXT:  $Reg0 @17 [empty]	%9 = ReturnInst %8 : string
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg0 @14 [15...16) 	%10 = HBCLoadConstInst "foo1" : string
// CHECK-NEXT:  $Reg0 @15 [empty]	%11 = ReturnInst %10 : string
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  $Reg0 @12 [13...14) 	%12 = HBCLoadConstInst "bar2" : string
// CHECK-NEXT:  $Reg0 @13 [empty]	%13 = ReturnInst %12 : string
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  $Reg0 @10 [11...12) 	%14 = HBCLoadConstInst "baz3" : string
// CHECK-NEXT:  $Reg0 @11 [empty]	%15 = ReturnInst %14 : string
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  $Reg0 @8 [9...10) 	%16 = HBCLoadConstInst "foo4" : string
// CHECK-NEXT:  $Reg0 @9 [empty]	%17 = ReturnInst %16 : string
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  $Reg0 @6 [7...8) 	%18 = HBCLoadConstInst "bar5" : string
// CHECK-NEXT:  $Reg0 @7 [empty]	%19 = ReturnInst %18 : string
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%20 = HBCLoadConstInst "baz6" : string
// CHECK-NEXT:  $Reg0 @5 [empty]	%21 = ReturnInst %20 : string
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  $Reg0 @2 [3...4) 	%22 = HBCLoadConstInst "baz9" : string
// CHECK-NEXT:  $Reg0 @3 [empty]	%23 = ReturnInst %22 : string
// CHECK-NEXT:function_end

// CHECK:function string_switch#0#1(x)#2 : undefined|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 @0 [1...8) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg0 @1 [empty]	%1 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @8 [9...10) 	%2 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @9 [empty]	%3 = ReturnInst %2 : undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 @14 [15...16) 	%4 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg0 @15 [empty]	%5 = ReturnInst %4 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg0 @12 [13...14) 	%6 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg0 @13 [empty]	%7 = ReturnInst %6 : number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg0 @10 [11...12) 	%8 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg0 @11 [empty]	%9 = ReturnInst %8 : number
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  $Reg0 @6 [7...8) 	%10 = HBCLoadConstInst "c" : string
// CHECK-NEXT:  $Reg0 @7 [empty]	%11 = CompareBranchInst '===', %10 : string, %0, %BB5, %BB2
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%12 = HBCLoadConstInst "b" : string
// CHECK-NEXT:  $Reg0 @5 [empty]	%13 = CompareBranchInst '===', %12 : string, %0, %BB4, %BB6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 @2 [3...4) 	%14 = HBCLoadConstInst "a" : string
// CHECK-NEXT:  $Reg0 @3 [empty]	%15 = CompareBranchInst '===', %14 : string, %0, %BB3, %BB7
// CHECK-NEXT:function_end

// CHECK:function switch_uint32#0#1(x)#2 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg0 @1 [empty]	%1 = SwitchImmInst %0, %BB1, 2147483648 : number, 10 : number, 2147483648 : number, %BB2, 2147483649 : number, %BB2, 2147483650 : number, %BB2, 2147483651 : number, %BB2, 2147483652 : number, %BB2, 2147483653 : number, %BB2, 2147483654 : number, %BB2, 2147483655 : number, %BB2, 2147483656 : number, %BB2, 2147483657 : number, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%2 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg0 @5 [empty]	%3 = ReturnInst %2 : number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @2 [3...4) 	%4 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg0 @3 [empty]	%5 = ReturnInst %4 : number
// CHECK-NEXT:function_end
