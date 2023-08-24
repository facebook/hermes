/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ra %s | %FileCheckOrRegen --match-full-lines %s
// Mov elimination incorrectly eliminated a mov in this case.

function fib(n) {
  var f0 = 0, f1 = 1;
  for (; n > 0; n = n -1) {
	var f2 = f0 + f1;
	f0 = f1; f1 = f2;
  }
  return f0;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:globals = [fib]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg1 @1 [2...4) 	%1 = HBCCreateFunctionInst %fib#0#1()#2 : string|number|bigint, %0
// CHECK-NEXT:  $Reg0 @2 [3...4) 	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg0 @3 [empty]	%3 = StorePropertyInst %1 : closure, %2 : object, "fib" : string
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%4 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @5 [empty]	%5 = ReturnInst %4 : undefined
// CHECK-NEXT:function_end

// CHECK:function fib#0#1(n)#2 : string|number|bigint
// CHECK-NEXT:S{fib#0#1()#2} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg5 @0 [1...5) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg4 @1 [2...20) 	%1 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg6 @2 [3...9) 	%2 = BinaryOperatorInst '>', %0, %1 : number
// CHECK-NEXT:  $Reg3 @3 [4...20) 	%3 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg5 @4 [5...10) 	%4 = MovInst %0
// CHECK-NEXT:  $Reg2 @5 [6...11) 	%5 = MovInst %1 : number
// CHECK-NEXT:  $Reg1 @6 [7...12) 	%6 = MovInst %3 : number
// CHECK-NEXT:  $Reg0 @7 [8...21) 	%7 = MovInst %5 : number
// CHECK-NEXT:  $Reg6 @8 [empty]	%8 = CondBranchInst %2 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg5 @9 [1...20) 	%9 = PhiInst %4, %BB0, %18 : number, %BB1
// CHECK-NEXT:  $Reg2 @10 [6...13) [16...20) 	%10 = PhiInst %5 : number, %BB0, %15 : string|number|bigint, %BB1
// CHECK-NEXT:  $Reg1 @11 [7...15) [17...20) 	%11 = PhiInst %6 : number, %BB0, %16 : string|number|bigint, %BB1
// CHECK-NEXT:  $Reg7 @12 [13...17) 	%12 = BinaryOperatorInst '+', %10 : string|number|bigint, %11 : string|number|bigint
// CHECK-NEXT:  $Reg5 @13 [14...19) 	%13 = BinaryOperatorInst '-', %9, %3 : number
// CHECK-NEXT:  $Reg6 @14 [15...18) 	%14 = MovInst %11 : string|number|bigint
// CHECK-NEXT:  $Reg2 @15 [16...19) 	%15 = MovInst %14 : string|number|bigint
// CHECK-NEXT:  $Reg1 @16 [17...19) 	%16 = MovInst %12 : string|number|bigint
// CHECK-NEXT:  $Reg0 @17 [18...21) 	%17 = MovInst %15 : string|number|bigint
// CHECK-NEXT:  $Reg5 @18 [19...20) 	%18 = MovInst %13 : number
// CHECK-NEXT:  $Reg1 @19 [empty]	%19 = CompareBranchInst '>', %18 : number, %1 : number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @20 [8...22) 	%20 = PhiInst %7 : number, %BB0, %17 : string|number|bigint, %BB1
// CHECK-NEXT:  $Reg0 @21 [8...23) 	%21 = MovInst %20 : string|number|bigint
// CHECK-NEXT:  $Reg0 @22 [empty]	%22 = ReturnInst %21 : string|number|bigint
// CHECK-NEXT:function_end
