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

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [fib]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCCreateEnvironmentInst
// CHECK-NEXT:  $Reg1 @1 [2...4) 	%1 = HBCCreateFunctionInst %fib() : number, %0
// CHECK-NEXT:  $Reg0 @2 [3...4) 	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg0 @3 [empty]	%3 = StorePropertyLooseInst %1 : closure, %2 : object, "fib" : string
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%4 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @5 [empty]	%5 = ReturnInst %4 : undefined
// CHECK-NEXT:function_end

// CHECK:function fib(n) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg5 @0 [1...6) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg4 @1 [2...19) 	%1 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg3 @2 [3...19) 	%2 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg2 @3 [4...9) 	%3 = MovInst %1 : number
// CHECK-NEXT:  $Reg1 @4 [5...10) 	%4 = MovInst %2 : number
// CHECK-NEXT:  $Reg5 @5 [6...11) 	%5 = MovInst %0
// CHECK-NEXT:  $Reg0 @6 [7...20) 	%6 = MovInst %3 : number
// CHECK-NEXT:  $Reg6 @7 [empty]	%7 = CompareBranchInst '>', %5, %6 : number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg2 @8 [4...12) [15...19) 	%8 = PhiInst %3 : number, %BB0, %14 : number, %BB1
// CHECK-NEXT:  $Reg1 @9 [5...14) [16...19) 	%9 = PhiInst %4 : number, %BB0, %15 : number, %BB1
// CHECK-NEXT:  $Reg5 @10 [1...19) 	%10 = PhiInst %5, %BB0, %17 : number, %BB1
// CHECK-NEXT:  $Reg7 @11 [12...16) 	%11 = BinaryOperatorInst '+', %8 : number, %9 : number
// CHECK-NEXT:  $Reg5 @12 [13...18) 	%12 = BinaryOperatorInst '-', %10, %2 : number
// CHECK-NEXT:  $Reg6 @13 [14...17) 	%13 = MovInst %9 : number
// CHECK-NEXT:  $Reg2 @14 [15...18) 	%14 = MovInst %13 : number
// CHECK-NEXT:  $Reg1 @15 [16...18) 	%15 = MovInst %11 : number
// CHECK-NEXT:  $Reg0 @16 [17...20) 	%16 = MovInst %14 : number
// CHECK-NEXT:  $Reg5 @17 [18...19) 	%17 = MovInst %12 : number
// CHECK-NEXT:  $Reg1 @18 [empty]	%18 = CompareBranchInst '>', %17 : number, %1 : number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @19 [7...21) 	%19 = PhiInst %6 : number, %BB0, %16 : number, %BB1
// CHECK-NEXT:  $Reg0 @20 [7...22) 	%20 = MovInst %19 : number
// CHECK-NEXT:  $Reg0 @21 [empty]	%21 = ReturnInst %20 : number
// CHECK-NEXT:function_end
