/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheckOrRegen --match-full-lines %s

function count() {
  return arguments.length;
}

function select(x) {
  return arguments[x+1];
}

function build() {
  return arguments;
}

function buffalobuffalo() {
  if(arguments) {
    return arguments + arguments;
  }
  return arguments;
}

function check_phi_handling(x) {
  return x ? [1] : arguments;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [count, select, build, buffalobuffalo, check_phi_handling]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...11) 	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg2 @1 [2...4) 	%1 = HBCCreateFunctionInst %count#0#1()#2, %0
// CHECK-NEXT:  $Reg1 @2 [3...12) 	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg2 @3 [empty]	%3 = StorePropertyInst %1 : closure, %2 : object, "count" : string
// CHECK-NEXT:  $Reg2 @4 [5...6) 	%4 = HBCCreateFunctionInst %select#0#1()#3, %0
// CHECK-NEXT:  $Reg2 @5 [empty]	%5 = StorePropertyInst %4 : closure, %2 : object, "select" : string
// CHECK-NEXT:  $Reg2 @6 [7...8) 	%6 = HBCCreateFunctionInst %build#0#1()#4 : object, %0
// CHECK-NEXT:  $Reg2 @7 [empty]	%7 = StorePropertyInst %6 : closure, %2 : object, "build" : string
// CHECK-NEXT:  $Reg2 @8 [9...10) 	%8 = HBCCreateFunctionInst %buffalobuffalo#0#1()#5 : string|number, %0
// CHECK-NEXT:  $Reg2 @9 [empty]	%9 = StorePropertyInst %8 : closure, %2 : object, "buffalobuffalo" : string
// CHECK-NEXT:  $Reg0 @10 [11...12) 	%10 = HBCCreateFunctionInst %check_phi_handling#0#1()#6 : object, %0
// CHECK-NEXT:  $Reg0 @11 [empty]	%11 = StorePropertyInst %10 : closure, %2 : object, "check_phi_handling" : string
// CHECK-NEXT:  $Reg0 @12 [13...14) 	%12 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @13 [empty]	%13 = ReturnInst %12 : undefined
// CHECK-NEXT:function_end

// CHECK:function count#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...4) 	%0 = AllocStackInst $arguments
// CHECK-NEXT:  $Reg1 @1 [2...3) 	%1 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg1 @2 [empty]	%2 = StoreStackInst %1 : undefined, %0
// CHECK-NEXT:  $Reg0 @3 [4...5) 	%3 = HBCGetArgumentsLengthInst %0
// CHECK-NEXT:  $Reg0 @4 [empty]	%4 = ReturnInst %3
// CHECK-NEXT:function_end

// CHECK:function select#0#1(x)#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 @0 [1...7) 	%0 = AllocStackInst $arguments
// CHECK-NEXT:  $Reg0 @1 [2...3) 	%1 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @2 [empty]	%2 = StoreStackInst %1 : undefined, %0
// CHECK-NEXT:  $Reg2 @3 [4...6) 	%3 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%4 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg0 @5 [6...7) 	%5 = BinaryOperatorInst '+', %3, %4 : number
// CHECK-NEXT:  $Reg0 @6 [7...8) 	%6 = HBCGetArgumentsPropByValInst %5 : string|number, %0
// CHECK-NEXT:  $Reg0 @7 [empty]	%7 = ReturnInst %6
// CHECK-NEXT:function_end

// CHECK:function build#0#1()#2 : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...5) 	%0 = AllocStackInst $arguments
// CHECK-NEXT:  $Reg1 @1 [2...3) 	%1 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg1 @2 [empty]	%2 = StoreStackInst %1 : undefined, %0
// CHECK-NEXT:  $Reg1 @3 [empty]	%3 = HBCReifyArgumentsInst %0
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%4 = LoadStackInst %0
// CHECK-NEXT:  $Reg0 @5 [empty]	%5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function buffalobuffalo#0#1()#2 : string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...5) 	%0 = AllocStackInst $arguments
// CHECK-NEXT:  $Reg1 @1 [2...3) 	%1 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg1 @2 [empty]	%2 = StoreStackInst %1 : undefined, %0
// CHECK-NEXT:  $Reg1 @3 [empty]	%3 = HBCReifyArgumentsInst %0
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%4 = LoadStackInst %0
// CHECK-NEXT:  $Reg0 @5 [6...7) 	%5 = BinaryOperatorInst '+', %4, %4
// CHECK-NEXT:  $Reg0 @6 [empty]	%6 = ReturnInst %5 : string|number
// CHECK-NEXT:function_end

// CHECK:function check_phi_handling#0#1(x)#2 : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...7) 	%0 = AllocStackInst $arguments
// CHECK-NEXT:  $Reg1 @1 [2...3) 	%1 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg1 @2 [empty]	%2 = StoreStackInst %1 : undefined, %0
// CHECK-NEXT:  $Reg1 @3 [4...5) 	%3 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg1 @4 [empty]	%4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg1 @9 [10...11) 	%5 = AllocArrayInst 1 : number, 1 : number
// CHECK-NEXT:  $Reg0 @10 [11...13) 	%6 = MovInst %5 : object
// CHECK-NEXT:  $Reg1 @11 [empty]	%7 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg0 @12 [7...14) 	%8 = PhiInst %6 : object, %BB1, %13, %BB2
// CHECK-NEXT:  $Reg0 @13 [7...15) 	%9 = MovInst %8 : object
// CHECK-NEXT:  $Reg0 @14 [empty]	%10 = ReturnInst %9 : object
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg1 @5 [empty]	%11 = HBCReifyArgumentsInst %0
// CHECK-NEXT:  $Reg0 @6 [7...8) 	%12 = LoadStackInst %0
// CHECK-NEXT:  $Reg0 @7 [8...13) 	%13 = MovInst %12
// CHECK-NEXT:  $Reg1 @8 [empty]	%14 = BranchInst %BB3
// CHECK-NEXT:function_end
