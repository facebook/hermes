/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheck %s --match-full-lines

//CHECK-LABEL:function global() : undefined|object
//CHECK-NEXT:frame = [], globals = [a, x, y, i]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCGetGlobalObjectInst
//CHECK-NEXT:  {{.*}}  %1 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  {{.*}}  %3 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadConstInst 2 : number
//CHECK-NEXT:  {{.*}}  %5 = AllocArrayInst 0 : number
//CHECK-NEXT:  {{.*}}  %6 = StorePropertyInst %5 : object, %0 : object, "a" : string
//CHECK-NEXT:  {{.*}}  %7 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:  {{.*}}  %8 = StorePropertyInst %7 : object, %0 : object, "x" : string
//CHECK-NEXT:  {{.*}}  %9 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:  {{.*}}  %10 = StorePropertyInst %9 : object, %0 : object, "y" : string
//CHECK-NEXT:  {{.*}}  %11 = HBCLoadConstInst 0 : number
//CHECK-NEXT:  {{.*}}  %12 = StorePropertyInst %11 : number, %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %13 = LoadPropertyInst %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %14 = MovInst %1 : undefined
//CHECK-NEXT:  {{.*}}  %15 = CompareBranchInst '<', %13, %3 : number, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %16 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:  {{.*}}  %17 = StorePropertyInst %16 : object, %0 : object, "y" : string
//CHECK-NEXT:  {{.*}}  %18 = AllocStackInst $?anon_1_iter
//CHECK-NEXT:  {{.*}}  %19 = AllocStackInst $?anon_2_base
//CHECK-NEXT:  {{.*}}  %20 = AllocStackInst $?anon_3_idx
//CHECK-NEXT:  {{.*}}  %21 = AllocStackInst $?anon_4_size
//CHECK-NEXT:  {{.*}}  %22 = LoadPropertyInst %0 : object, "x" : string
//CHECK-NEXT:  {{.*}}  %23 = StoreStackInst %22, %19
//CHECK-NEXT:  {{.*}}  %24 = AllocStackInst $?anon_5_prop
//CHECK-NEXT:  {{.*}}  %25 = GetPNamesInst %18, %19, %20, %21, %BB3, %BB4
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %26 = PhiInst %14 : undefined, %BB0, %34 : object, %BB3
//CHECK-NEXT:  {{.*}}  %27 = MovInst %26 : undefined|object
//CHECK-NEXT:  {{.*}}  %28 = ReturnInst %27 : undefined|object
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  {{.*}}  %29 = LoadPropertyInst %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %30 = AsNumberInst %29
//CHECK-NEXT:  {{.*}}  %31 = BinaryOperatorInst '+', %30 : number, %2 : number
//CHECK-NEXT:  {{.*}}  %32 = StorePropertyInst %31 : number, %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %33 = LoadPropertyInst %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %34 = MovInst %16 : object
//CHECK-NEXT:  {{.*}}  %35 = CompareBranchInst '<', %33, %3 : number, %BB1, %BB2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  {{.*}}  %36 = GetNextPNameInst %24, %19, %20, %21, %18, %BB3, %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  {{.*}}  %37 = LoadStackInst %24
//CHECK-NEXT:  {{.*}}  %38 = LoadPropertyInst %0 : object, "a" : string
//CHECK-NEXT:  {{.*}}  %39 = StorePropertyInst %37, %38, %4 : number
//CHECK-NEXT:  {{.*}}  %40 = BranchInst %BB4
//CHECK-NEXT:function_end


var a = [];
var x = {};
var y = {}

for (var i=0 ; i < 3; ++i) {
  y = {}

  for (a[2] in x) {
  }
}
