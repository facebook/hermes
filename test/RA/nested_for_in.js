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
// CHECK-NEXT:globals = [a, x, y, i]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 @0 [1...3) 	%0 = AllocArrayInst 0 : number
// CHECK-NEXT:  $Reg4 @1 [2...37) 	%1 = HBCGetGlobalObjectInst
// CHECK-NEXT:  $Reg0 @2 [empty]	%2 = StorePropertyInst %0 : object, %1 : object, "a" : string
// CHECK-NEXT:  $Reg0 @3 [4...5) 	%3 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  $Reg0 @4 [empty]	%4 = StorePropertyInst %3 : object, %1 : object, "x" : string
// CHECK-NEXT:  $Reg0 @5 [6...7) 	%5 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  $Reg0 @6 [empty]	%6 = StorePropertyInst %5 : object, %1 : object, "y" : string
// CHECK-NEXT:  $Reg0 @7 [8...9) 	%7 = HBCLoadConstInst 0 : number
// CHECK-NEXT:  $Reg0 @8 [empty]	%8 = StorePropertyInst %7 : number, %1 : object, "i" : string
// CHECK-NEXT:  $Reg0 @9 [10...12) 	%9 = LoadPropertyInst %1 : object, "i" : string
// CHECK-NEXT:  $Reg3 @10 [11...37) 	%10 = HBCLoadConstInst 3 : number
// CHECK-NEXT:  $Reg1 @11 [12...16) 	%11 = BinaryOperatorInst '<', %9, %10 : number
// CHECK-NEXT:  $Reg0 @12 [13...15) 	%12 = HBCLoadConstInst undefined : undefined
// CHECK-NEXT:  $Reg2 @13 [14...37) 	%13 = HBCLoadConstInst 2 : number
// CHECK-NEXT:  $Reg0 @14 [15...38) 	%14 = MovInst %12 : undefined
// CHECK-NEXT:  $Reg1 @15 [empty]	%15 = CondBranchInst %11 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg5 @16 [17...36) 	%16 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  $Reg1 @17 [empty]	%17 = StorePropertyInst %16 : object, %1 : object, "y" : string
// CHECK-NEXT:  $Reg9 @18 [19...31) 	%18 = AllocStackInst $?anon_1_iter
// CHECK-NEXT:  $Reg8 @19 [20...31) 	%19 = AllocStackInst $?anon_2_base
// CHECK-NEXT:  $Reg7 @20 [21...31) 	%20 = AllocStackInst $?anon_3_idx
// CHECK-NEXT:  $Reg6 @21 [22...31) 	%21 = AllocStackInst $?anon_4_size
// CHECK-NEXT:  $Reg1 @22 [23...24) 	%22 = LoadPropertyInst %1 : object, "x" : string
// CHECK-NEXT:  $Reg1 @23 [empty]	%23 = StoreStackInst %22, %19
// CHECK-NEXT:  $Reg1 @24 [25...31) 	%24 = AllocStackInst $?anon_5_prop
// CHECK-NEXT:  $Reg10 @25 [empty]	%25 = GetPNamesInst %18, %19, %20, %21, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 @37 [13...39) 	%26 = PhiInst %14 : undefined, %BB0, %33 : object, %BB3
// CHECK-NEXT:  $Reg0 @38 [13...40) 	%27 = MovInst %26 : undefined|object
// CHECK-NEXT:  $Reg0 @39 [empty]	%28 = ReturnInst %27 : undefined|object
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg1 @31 [32...33) 	%29 = LoadPropertyInst %1 : object, "i" : string
// CHECK-NEXT:  $Reg1 @32 [33...34) 	%30 = UnaryOperatorInst '++', %29
// CHECK-NEXT:  $Reg1 @33 [empty]	%31 = StorePropertyInst %30 : number|bigint, %1 : object, "i" : string
// CHECK-NEXT:  $Reg1 @34 [35...37) 	%32 = LoadPropertyInst %1 : object, "i" : string
// CHECK-NEXT:  $Reg0 @35 [36...38) 	%33 = MovInst %16 : object
// CHECK-NEXT:  $Reg1 @36 [empty]	%34 = CompareBranchInst '<', %32, %10 : number, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  $Reg10 @26 [empty]	%35 = GetNextPNameInst %24, %19, %20, %21, %18, %BB3, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  $Reg11 @27 [28...30) 	%36 = LoadStackInst %24
// CHECK-NEXT:  $Reg10 @28 [29...30) 	%37 = LoadPropertyInst %1 : object, "a" : string
// CHECK-NEXT:  $Reg10 @29 [empty]	%38 = StorePropertyInst %36, %37, %13 : number
// CHECK-NEXT:  $Reg1 @30 [empty]	%39 = BranchInst %BB4
// CHECK-NEXT:function_end
