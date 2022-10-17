/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheckOrRegen %s --match-full-lines

var a = [];
var x = {};
var y = {}

for (var i=0 ; i < 3; ++i) {
  y = {}

  for (a[2] in x) {
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined|object
// CHECK-NEXT:frame = [], globals = [a, x, y, i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg4 @0 [1...36) 	%0 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg0 @1 [2...14) 	%1 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg3 @2 [3...36) 	%2 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg2 @3 [4...36) 	%3 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg1 @4 [5...6) 	%4 = AllocArrayInst 0 : number
// CHECK-NEXT:  $Reg1 @5 [empty]	%5 = StorePropertyInst %4 : object, %0 : object, "a" : string
// CHECK-NEXT:  $Reg1 @6 [7...8) 	%6 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  $Reg1 @7 [empty]	%7 = StorePropertyInst %6 : object, %0 : object, "x" : string
// CHECK-NEXT:  $Reg1 @8 [9...10) 	%8 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  $Reg1 @9 [empty]	%9 = StorePropertyInst %8 : object, %0 : object, "y" : string
// CHECK-NEXT:  $Reg1 @10 [11...12) 	%10 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg1 @11 [empty]	%11 = StorePropertyInst %10 : number, %0 : object, "i" : string
// CHECK-NEXT:  $Reg1 @12 [13...15) 	%12 = LoadPropertyInst %0 : object, "i" : string
// CHECK-NEXT:  $Reg0 @13 [14...37) 	%13 = MovInst %1 : undefined
// CHECK-NEXT:  $Reg1 @14 [empty]	%14 = CompareBranchInst '<', %12, %2 : number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg5 @15 [16...35) 	%15 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  $Reg1 @16 [empty]	%16 = StorePropertyInst %15 : object, %0 : object, "y" : string
// CHECK-NEXT:  $Reg9 @17 [18...30) 	%17 = AllocStackInst $?anon_1_iter
// CHECK-NEXT:  $Reg8 @18 [19...30) 	%18 = AllocStackInst $?anon_2_base
// CHECK-NEXT:  $Reg7 @19 [20...30) 	%19 = AllocStackInst $?anon_3_idx
// CHECK-NEXT:  $Reg6 @20 [21...30) 	%20 = AllocStackInst $?anon_4_size
// CHECK-NEXT:  $Reg1 @21 [22...23) 	%21 = LoadPropertyInst %0 : object, "x" : string
// CHECK-NEXT:  $Reg1 @22 [empty]	%22 = StoreStackInst %21, %18
// CHECK-NEXT:  $Reg1 @23 [24...30) 	%23 = AllocStackInst $?anon_5_prop
// CHECK-NEXT:  $Reg10 @24 [empty]	%24 = GetPNamesInst %17, %18, %19, %20, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @36 [2...38) 	%25 = PhiInst %13 : undefined, %BB0, %32 : object, %BB3
// CHECK-NEXT:  $Reg0 @37 [2...39) 	%26 = MovInst %25 : undefined|object
// CHECK-NEXT:  $Reg0 @38 [empty]	%27 = ReturnInst %26 : undefined|object
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg1 @30 [31...32) 	%28 = LoadPropertyInst %0 : object, "i" : string
// CHECK-NEXT:  $Reg1 @31 [32...33) 	%29 = UnaryOperatorInst '++', %28
// CHECK-NEXT:  $Reg1 @32 [empty]	%30 = StorePropertyInst %29 : number|bigint, %0 : object, "i" : string
// CHECK-NEXT:  $Reg1 @33 [34...36) 	%31 = LoadPropertyInst %0 : object, "i" : string
// CHECK-NEXT:  $Reg0 @34 [35...37) 	%32 = MovInst %15 : object
// CHECK-NEXT:  $Reg1 @35 [empty]	%33 = CompareBranchInst '<', %31, %2 : number, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg10 @25 [empty]	%34 = GetNextPNameInst %23, %18, %19, %20, %17, %BB3, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg11 @26 [27...29) 	%35 = LoadStackInst %23
// CHECK-NEXT:  $Reg10 @27 [28...29) 	%36 = LoadPropertyInst %0 : object, "a" : string
// CHECK-NEXT:  $Reg10 @28 [empty]	%37 = StorePropertyInst %35, %36, %3 : number
// CHECK-NEXT:  $Reg1 @29 [empty]	%38 = BranchInst %BB4
// CHECK-NEXT:function_end
