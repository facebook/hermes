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
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  {{.*}}  %3 = HBCLoadConstInst 2 : number
//CHECK-NEXT:  {{.*}}  %4 = AllocArrayInst 0 : number
//CHECK-NEXT:  {{.*}}  %5 = StorePropertyInst %4 : object, %0 : object, "a" : string
//CHECK-NEXT:  {{.*}}  %6 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:  {{.*}}  %7 = StorePropertyInst %6 : object, %0 : object, "x" : string
//CHECK-NEXT:  {{.*}}  %8 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:  {{.*}}  %9 = StorePropertyInst %8 : object, %0 : object, "y" : string
//CHECK-NEXT:  {{.*}}  %10 = HBCLoadConstInst 0 : number
//CHECK-NEXT:  {{.*}}  %11 = StorePropertyInst %10 : number, %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %12 = LoadPropertyInst %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %13 = MovInst %1 : undefined
//CHECK-NEXT:  {{.*}}  %14 = CompareBranchInst '<', %12, %2 : number, %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}}  %15 = AllocObjectInst 0 : number, empty
//CHECK-NEXT:  {{.*}}  %16 = StorePropertyInst %15 : object, %0 : object, "y" : string
//CHECK-NEXT:  {{.*}}  %17 = AllocStackInst $?anon_1_iter
//CHECK-NEXT:  {{.*}}  %18 = AllocStackInst $?anon_2_base
//CHECK-NEXT:  {{.*}}  %19 = AllocStackInst $?anon_3_idx
//CHECK-NEXT:  {{.*}}  %20 = AllocStackInst $?anon_4_size
//CHECK-NEXT:  {{.*}}  %21 = LoadPropertyInst %0 : object, "x" : string
//CHECK-NEXT:  {{.*}}  %22 = StoreStackInst %21, %18
//CHECK-NEXT:  {{.*}}  %23 = AllocStackInst $?anon_5_prop
//CHECK-NEXT:  {{.*}}  %24 = GetPNamesInst %17, %18, %19, %20, %BB3, %BB4
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}}  %25 = PhiInst %13 : undefined, %BB0, %32 : object, %BB3
//CHECK-NEXT:  {{.*}}  %26 = MovInst %25 : undefined|object
//CHECK-NEXT:  {{.*}}  %27 = ReturnInst %26 : undefined|object
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  {{.*}}  %28 = LoadPropertyInst %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %29 = UnaryOperatorInst '++', %28
//CHECK-NEXT:  {{.*}}  %30 = StorePropertyInst %29 : number|bigint, %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %31 = LoadPropertyInst %0 : object, "i" : string
//CHECK-NEXT:  {{.*}}  %32 = MovInst %15 : object
//CHECK-NEXT:  {{.*}}  %33 = CompareBranchInst '<', %31, %2 : number, %BB1, %BB2
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  {{.*}}  %34 = GetNextPNameInst %23, %18, %19, %20, %17, %BB3, %BB5
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  {{.*}}  %35 = LoadStackInst %23
//CHECK-NEXT:  {{.*}}  %36 = LoadPropertyInst %0 : object, "a" : string
//CHECK-NEXT:  {{.*}}  %37 = StorePropertyInst %35, %36, %3 : number
//CHECK-NEXT:  {{.*}}  %38 = BranchInst %BB4
//CHECK-NEXT:function_end


var a = [];
var x = {};
var y = {}

for (var i=0 ; i < 3; ++i) {
  y = {}

  for (a[2] in x) {
  }
}
