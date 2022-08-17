/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:function f(x) : string
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   {{.*}} 	%0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:   {{.*}} 	%1 = HBCLoadConstInst "fall" : string
//CHECK-NEXT:   {{.*}} 	%2 = HBCLoadConstInst "" : string
//CHECK-NEXT:   $Reg2 @3 [empty]	%3 = BranchInst %BB1
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   {{.*}} 	%4 = HBCLoadConstInst "regular" : string
//CHECK-NEXT:   $Reg0 @26 [empty]	%5 = ReturnInst %4 : string
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   {{.*}} 	%6 = HBCLoadConstInst "multicase" : string
//CHECK-NEXT:   $Reg0 @24 [empty]	%7 = ReturnInst %6 : string
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   {{.*}} 	%8 = MovInst %1 : string
//CHECK-NEXT:   $Reg0 @18 [empty]	%9 = BranchInst %BB5
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   {{.*}} 	%10 = PhiInst %8 : string, %BB4, %17 : string, %BB6
//CHECK-NEXT:   {{.*}} 	%11 = HBCLoadConstInst "through" : string
//CHECK-NEXT:   {{.*}} 	%12 = BinaryOperatorInst '+', %10 : string, %11 : string
//CHECK-NEXT:   $Reg0 @22 [empty]	%13 = ReturnInst %12 : string
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   {{.*}} 	%14 = HBCLoadConstInst "default" : string
//CHECK-NEXT:   $Reg2 @16 [empty]	%15 = ReturnInst %14 : string
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   {{.*}} 	%16 = HBCLoadConstInst 4 : number
//CHECK-NEXT:   {{.*}} 	%17 = MovInst %2 : string
//CHECK-NEXT:   $Reg2 @14 [empty]	%18 = CompareBranchInst '===', %16 : number, %0, %BB5, %BB7
//CHECK-NEXT: %BB8:
//CHECK-NEXT:   {{.*}} 	%19 = HBCLoadConstInst 3 : number
//CHECK-NEXT:   $Reg2 @11 [empty]	%20 = CompareBranchInst '===', %19 : number, %0, %BB4, %BB6
//CHECK-NEXT: %BB9:
//CHECK-NEXT:   {{.*}} 	%21 = HBCLoadConstInst 2 : number
//CHECK-NEXT:   $Reg2 @9 [empty]	%22 = CompareBranchInst '===', %21 : number, %0, %BB3, %BB8
//CHECK-NEXT: %BB10:
//CHECK-NEXT:   {{.*}} 	%23 = HBCLoadConstInst 1 : number
//CHECK-NEXT:   $Reg2 @7 [empty]	%24 = CompareBranchInst '===', %23 : number, %0, %BB3, %BB9
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   {{.*}} 	%25 = HBCLoadConstInst 0 : number
//CHECK-NEXT:   $Reg2 @5 [empty]	%26 = CompareBranchInst '===', %25 : number, %0, %BB2, %BB10
//CHECK-NEXT: function_end

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

//CHECK-LABEL:function regress1(w) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}} 	%0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}} 	%1 = HBCLoadConstInst 0 : number
//CHECK-NEXT:  {{.*}} 	%2 = HBCLoadConstInst 2 : number
//CHECK-NEXT:  {{.*}} 	%3 = HBCGetGlobalObjectInst
//CHECK-NEXT:  {{.*}} 	%4 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}} 	%5 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  {{.*}} 	%6 = MovInst %1 : number
//CHECK-NEXT:  $Reg7 @7 [empty]	%7 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}} 	%8 = PhiInst %6 : number, %BB0, %12 : number, %BB2, %14 : number, %BB3
//CHECK-NEXT:  {{.*}} 	%9 = TryLoadGlobalPropertyInst %3 : object, "print" : string
//CHECK-NEXT:  $Reg7 @10 [empty]	%10 = HBCCallNInst %9, %4 : undefined, %8 : number
//CHECK-NEXT:  $Reg7 @11 [empty]	%11 = BranchInst %BB4
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}} 	%12 = MovInst %2 : number
//CHECK-NEXT:  $Reg0 @16 [empty]	%13 = BranchInst %BB1
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  {{.*}} 	%14 = MovInst %8 : number
//CHECK-NEXT:  $Reg7 @14 [empty]	%15 = CompareBranchInst '===', %1 : number, %0, %BB2, %BB1
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  $Reg7 @12 [empty]	%16 = CompareBranchInst '===', %5 : number, %0, %BB2, %BB3

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


//CHECK-LABEL:function jump_table(x) : undefined|string
//CHECK-NEXT: frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}} 	%0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  $Reg0 @1 [empty]	%1 = SwitchImmInst %0, %BB1, 0 : number, 11 : number, 0 : number, %BB2, 1 : number, %BB3, 2 : number, %BB4, 3 : number, %BB5, 4 : number, %BB6, 5 : number, %BB7, 6 : number, %BB8, 8 : number, %BB9, 9 : number, %BB10, 10 : number, %BB11
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}} 	%2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  $Reg0 @23 [empty]	%3 = ReturnInst %2 : undefined
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}} 	%4 = HBCLoadConstInst "foo" : string
//CHECK-NEXT:  $Reg0 @21 [empty]	%5 = ReturnInst %4 : string
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  {{.*}} 	%6 = HBCLoadConstInst "bar" : string
//CHECK-NEXT:  $Reg0 @19 [empty]	%7 = ReturnInst %6 : string
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  {{.*}} 	%8 = HBCLoadConstInst "baz" : string
//CHECK-NEXT:  $Reg0 @17 [empty]	%9 = ReturnInst %8 : string
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  {{.*}} 	%10 = HBCLoadConstInst "foo1" : string
//CHECK-NEXT:  $Reg0 @15 [empty]	%11 = ReturnInst %10 : string
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  {{.*}} 	%12 = HBCLoadConstInst "bar2" : string
//CHECK-NEXT:  $Reg0 @13 [empty]	%13 = ReturnInst %12 : string
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  {{.*}} 	%14 = HBCLoadConstInst "baz3" : string
//CHECK-NEXT:  $Reg0 @11 [empty]	%15 = ReturnInst %14 : string
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  {{.*}} 	%16 = HBCLoadConstInst "foo4" : string
//CHECK-NEXT:  $Reg0 @9 [empty]	%17 = ReturnInst %16 : string
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  {{.*}} 	%18 = HBCLoadConstInst "bar5" : string
//CHECK-NEXT:  $Reg0 @7 [empty]	%19 = ReturnInst %18 : string
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  {{.*}} 	%20 = HBCLoadConstInst "baz6" : string
//CHECK-NEXT:  $Reg0 @5 [empty]	%21 = ReturnInst %20 : string
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  {{.*}} 	%22 = HBCLoadConstInst "baz9" : string
//CHECK-NEXT:  $Reg0 @3 [empty]	%23 = ReturnInst %22 : string
//CHECK-NEXT:function_end

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

//CHECK-LABEL:function string_switch(x) : undefined|number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   {{.*}} 	%0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:   $Reg0 @1 [empty]	%1 = BranchInst %BB1
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   {{.*}} 	%2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:   $Reg0 @9 [empty]	%3 = ReturnInst %2 : undefined
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   {{.*}} 	%4 = HBCLoadConstInst 1 : number
//CHECK-NEXT:   $Reg0 @15 [empty]	%5 = ReturnInst %4 : number
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   {{.*}} 	%6 = HBCLoadConstInst 2 : number
//CHECK-NEXT:   $Reg0 @13 [empty]	%7 = ReturnInst %6 : number
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   {{.*}} 	%8 = HBCLoadConstInst 3 : number
//CHECK-NEXT:   $Reg0 @11 [empty]	%9 = ReturnInst %8 : number
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   {{.*}} 	%10 = HBCLoadConstInst "c" : string
//CHECK-NEXT:   $Reg0 @7 [empty]	%11 = CompareBranchInst '===', %10 : string, %0, %BB5, %BB2
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   {{.*}} 	%12 = HBCLoadConstInst "b" : string
//CHECK-NEXT:   $Reg0 @5 [empty]	%13 = CompareBranchInst '===', %12 : string, %0, %BB4, %BB6
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   {{.*}} 	%14 = HBCLoadConstInst "a" : string
//CHECK-NEXT:   $Reg0 @3 [empty]	%15 = CompareBranchInst '===', %14 : string, %0, %BB3, %BB7
//CHECK-NEXT: function_end

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

// CHECK: function switch_uint32(x) : number
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   $Reg0 @0 [1...2) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:   $Reg0 @1 [empty]	%1 = SwitchImmInst %0, %BB1, 2147483648 : number, 10 : number, 2147483648 : number, %BB2, 2147483649 : number, %BB2, 2147483650 : number, %BB2, 2147483651 : number, %BB2, 2147483652 : number, %BB2, 2147483653 : number, %BB2, 2147483654 : number, %BB2, 2147483655 : number, %BB2, 2147483656 : number, %BB2, 2147483657 : number, %BB2
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   $Reg0 @4 [5...6) 	%2 = HBCLoadConstInst 1 : number
// CHECK-NEXT:   $Reg0 @5 [empty]	%3 = ReturnInst %2 : number
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   $Reg0 @2 [3...4) 	%4 = HBCLoadConstInst 0 : number
// CHECK-NEXT:   $Reg0 @3 [empty]	%5 = ReturnInst %4 : number
// CHECK-NEXT: function_end

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
