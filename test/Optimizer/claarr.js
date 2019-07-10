// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -enable-cla -dump-ir %s -O | %FileCheck %s --match-full-lines

//CHECK-LABEL:function module1(i, j)
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = AllocArrayInst 0 : number
//CHECK-NEXT:%1 = AllocObjectInst 1 : number, empty
//CHECK-NEXT:%2 = CreateFunctionInst %m() : boolean
//CHECK-NEXT:%3 = StoreNewOwnPropertyInst %2 : closure, %1 : object, "m" : string, true : boolean
//CHECK-NEXT:%4 = StorePropertyInst %1 : object, %0 : object, %i
//CHECK-NEXT:%5 = LoadPropertyInst %0 : object, %j
//CHECK-NEXT:%6 = LoadPropertyInst %5, "m" : string
//CHECK-NEXT:%7 = CallInst %6, %5
//CHECK-NEXT:%8 = ReturnInst %7
//CHECK-NEXT:function_end
function module1(i,j) {
    var a = [];
    a[i] = { m : function() { return true;} };
    var b = a[j];
    return b.m();
}

//CHECK-LABEL:function module2()
//CHECK-NEXT:frame = []
//CHECK-NEXT:    %BB0:
//CHECK-NEXT:%0 = AllocArrayInst 1 : number, 1 : number
//CHECK-NEXT:%1 = LoadPropertyInst %0 : object, 1 : number
//CHECK-NEXT:%2 = ReturnInst %1
//CHECK-NEXT:function_end
function module2() {
    var a = [1];
    return a[1];
}
