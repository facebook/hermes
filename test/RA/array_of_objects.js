/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O -target=HBC %s | %FileCheckOrRegen %s --match-full-lines
var arr = [{a: 1}, {b: 2}, {c: 3}, {d: 4}];

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [arr]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...5) 	%0 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  $Reg1 @1 [2...3) 	%1 = HBCLoadConstInst 1 : number
// CHECK-NEXT:  $Reg1 @2 [empty]	%2 = StoreNewOwnPropertyInst %1 : number, %0 : object, "a" : string, true : boolean
// CHECK-NEXT:  $Reg1 @3 [4...19) 	%3 = AllocArrayInst 4 : number
// CHECK-NEXT:  $Reg0 @4 [empty]	%4 = StoreOwnPropertyInst %0 : object, %3 : object, 0 : number, true : boolean
// CHECK-NEXT:  $Reg0 @5 [6...9) 	%5 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  $Reg2 @6 [7...8) 	%6 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg2 @7 [empty]	%7 = StoreNewOwnPropertyInst %6 : number, %5 : object, "b" : string, true : boolean
// CHECK-NEXT:  $Reg0 @8 [empty]	%8 = StoreOwnPropertyInst %5 : object, %3 : object, 1 : number, true : boolean
// CHECK-NEXT:  $Reg0 @9 [10...13) 	%9 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  $Reg2 @10 [11...12) 	%10 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg2 @11 [empty]	%11 = StoreNewOwnPropertyInst %10 : number, %9 : object, "c" : string, true : boolean
// CHECK-NEXT:  $Reg0 @12 [empty]	%12 = StoreOwnPropertyInst %9 : object, %3 : object, 2 : number, true : boolean
// CHECK-NEXT:  $Reg0 @13 [14...17) 	%13 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  $Reg2 @14 [15...16) 	%14 = HBCLoadConstInst 4 : number
// CHECK-NEXT:  $Reg2 @15 [empty]	%15 = StoreNewOwnPropertyInst %14 : number, %13 : object, "d" : string, true : boolean
// CHECK-NEXT:  $Reg0 @16 [empty]	%16 = StoreOwnPropertyInst %13 : object, %3 : object, 3 : number, true : boolean
// CHECK-NEXT:  $Reg0 @17 [18...19) 	%17 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg0 @18 [empty]	%18 = StorePropertyInst %3 : object, %17 : object, "arr" : string
// CHECK-NEXT:  $Reg0 @19 [20...21) 	%19 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg0 @20 [empty]	%20 = ReturnInst %19 : undefined
// CHECK-NEXT:function_end
