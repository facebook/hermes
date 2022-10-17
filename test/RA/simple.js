/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheckOrRegen %s --match-full-lines

function main(x, y, z) {

  var sum = 3;
  for (var i = 0; i < 10; i++) {
    sum += (x + i) * (y + i);
  }

  return sum;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [main]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...2) 	%0 = HBCCreateEnvironmentInst %S{global#0()#1}
// CHECK-NEXT:  $Reg1 @1 [2...4) 	%1 = HBCCreateFunctionInst %main#0#1()#2 : string|number|bigint, %0
// CHECK-NEXT:  $Reg0 @2 [3...4) 	%2 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg0 @3 [empty]	%3 = StorePropertyInst %1 : closure, %2 : object, "main" : string
// CHECK-NEXT:  $Reg0 @4 [5...6) 	%4 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @5 [empty]	%5 = ReturnInst %4 : undefined
// CHECK-NEXT:function_end

// CHECK:function main#0#1(x, y, z)#2 : string|number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg5 @0 [1...18) 	%0 = HBCLoadParamInst 1 : number
// CHECK-NEXT:  $Reg4 @1 [2...18) 	%1 = HBCLoadParamInst 2 : number
// CHECK-NEXT:  $Reg3 @2 [3...6) 	%2 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg2 @3 [4...7) 	%3 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg1 @4 [5...18) 	%4 = HBCLoadConstInst 10 : number
// CHECK-NEXT:  $Reg3 @5 [6...9) 	%5 = MovInst %2 : number
// CHECK-NEXT:  $Reg2 @6 [7...10) 	%6 = MovInst %3 : number
// CHECK-NEXT:  $Reg0 @7 [empty]	%7 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg3 @8 [3...14) [16...18) 	%8 = PhiInst %5 : number, %BB0, %15 : string|number|bigint, %BB1
// CHECK-NEXT:  $Reg2 @9 [4...18) 	%9 = PhiInst %6 : number, %BB0, %16 : number|bigint, %BB1
// CHECK-NEXT:  $Reg6 @10 [11...13) 	%10 = BinaryOperatorInst '+', %0, %9 : number|bigint
// CHECK-NEXT:  $Reg0 @11 [12...13) 	%11 = BinaryOperatorInst '+', %1, %9 : number|bigint
// CHECK-NEXT:  $Reg0 @12 [13...14) 	%12 = BinaryOperatorInst '*', %10 : string|number|bigint, %11 : string|number|bigint
// CHECK-NEXT:  $Reg0 @13 [14...19) 	%13 = BinaryOperatorInst '+', %8 : string|number|bigint, %12 : number|bigint
// CHECK-NEXT:  $Reg2 @14 [15...17) 	%14 = UnaryOperatorInst '++', %9 : number|bigint
// CHECK-NEXT:  $Reg3 @15 [16...17) 	%15 = MovInst %13 : string|number|bigint
// CHECK-NEXT:  $Reg2 @16 [17...18) 	%16 = MovInst %14 : number|bigint
// CHECK-NEXT:  $Reg1 @17 [empty]	%17 = CompareBranchInst '<', %16 : number|bigint, %4 : number, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @18 [empty]	%18 = ReturnInst %13 : string|number|bigint
// CHECK-NEXT:function_end
