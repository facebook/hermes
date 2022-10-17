/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ra %s | %FileCheckOrRegen %s --match-full-lines

var const_array = [1, 2, 3, "a"];

var t = 1;

var exp_array = [t, 1, t + 1, 2, t + 3];

var elision_array = [,,,"b"];

var const_then_elision_array = [1, 2, 3,,,"b"];

var exp_then_elision_array = [1, 2, t, t + 1,,,"b"];

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [const_array, t, exp_array, elision_array, const_then_elision_array, exp_then_elision_array]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...3) 	%0 = AllocArrayInst 4 : number, 1 : number, 2 : number, 3 : number, "a" : string
// CHECK-NEXT:  $Reg1 @1 [2...34) 	%1 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg0 @2 [empty]	%2 = StorePropertyInst %0 : object, %1 : object, "const_array" : string
// CHECK-NEXT:  $Reg4 @3 [4...31) 	%3 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg0 @4 [empty]	%4 = StorePropertyInst %3 : number, %1 : object, "t" : string
// CHECK-NEXT:  $Reg2 @5 [6...8) 	%5 = LoadPropertyInst %1 : object, "t" : string
// CHECK-NEXT:  $Reg0 @6 [7...19) 	%6 = AllocArrayInst 5 : number
// CHECK-NEXT:  $Reg2 @7 [empty]	%7 = StoreOwnPropertyInst %5, %6 : object, 0 : number, true : boolean
// CHECK-NEXT:  $Reg2 @8 [empty]	%8 = StoreOwnPropertyInst %3 : number, %6 : object, 1 : number, true : boolean
// CHECK-NEXT:  $Reg2 @9 [10...11) 	%9 = LoadPropertyInst %1 : object, "t" : string
// CHECK-NEXT:  $Reg2 @10 [11...12) 	%10 = BinaryOperatorInst '+', %9, %3 : number
// CHECK-NEXT:  $Reg2 @11 [empty]	%11 = StoreOwnPropertyInst %10 : string|number, %6 : object, 2 : number, true : boolean
// CHECK-NEXT:  $Reg2 @12 [13...14) 	%12 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg2 @13 [empty]	%13 = StoreOwnPropertyInst %12 : number, %6 : object, 3 : number, true : boolean
// CHECK-NEXT:  $Reg3 @14 [15...17) 	%14 = LoadPropertyInst %1 : object, "t" : string
// CHECK-NEXT:  $Reg2 @15 [16...17) 	%15 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg2 @16 [17...18) 	%16 = BinaryOperatorInst '+', %14, %15 : number
// CHECK-NEXT:  $Reg2 @17 [empty]	%17 = StoreOwnPropertyInst %16 : string|number, %6 : object, 4 : number, true : boolean
// CHECK-NEXT:  $Reg0 @18 [empty]	%18 = StorePropertyInst %6 : object, %1 : object, "exp_array" : string
// CHECK-NEXT:  $Reg0 @19 [20...23) 	%19 = AllocArrayInst 4 : number
// CHECK-NEXT:  $Reg2 @20 [21...33) 	%20 = HBCLoadConstInst "b" : string
// CHECK-NEXT:  $Reg3 @21 [empty]	%21 = StoreOwnPropertyInst %20 : string, %19 : object, 3 : number, true : boolean
// CHECK-NEXT:  $Reg0 @22 [empty]	%22 = StorePropertyInst %19 : object, %1 : object, "elision_array" : string
// CHECK-NEXT:  $Reg0 @23 [24...26) 	%23 = AllocArrayInst 6 : number, 1 : number, 2 : number, 3 : number
// CHECK-NEXT:  $Reg3 @24 [empty]	%24 = StoreOwnPropertyInst %20 : string, %23 : object, 5 : number, true : boolean
// CHECK-NEXT:  $Reg0 @25 [empty]	%25 = StorePropertyInst %23 : object, %1 : object, "const_then_elision_array" : string
// CHECK-NEXT:  $Reg3 @26 [27...29) 	%26 = LoadPropertyInst %1 : object, "t" : string
// CHECK-NEXT:  $Reg0 @27 [28...34) 	%27 = AllocArrayInst 7 : number, 1 : number, 2 : number
// CHECK-NEXT:  $Reg3 @28 [empty]	%28 = StoreOwnPropertyInst %26, %27 : object, 2 : number, true : boolean
// CHECK-NEXT:  $Reg3 @29 [30...31) 	%29 = LoadPropertyInst %1 : object, "t" : string
// CHECK-NEXT:  $Reg3 @30 [31...32) 	%30 = BinaryOperatorInst '+', %29, %3 : number
// CHECK-NEXT:  $Reg3 @31 [empty]	%31 = StoreOwnPropertyInst %30 : string|number, %27 : object, 3 : number, true : boolean
// CHECK-NEXT:  $Reg2 @32 [empty]	%32 = StoreOwnPropertyInst %20 : string, %27 : object, 6 : number, true : boolean
// CHECK-NEXT:  $Reg0 @33 [empty]	%33 = StorePropertyInst %27 : object, %1 : object, "exp_then_elision_array" : string
// CHECK-NEXT:  $Reg0 @34 [35...36) 	%34 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @35 [empty]	%35 = ReturnInst %34 : undefined
// CHECK-NEXT:function_end
