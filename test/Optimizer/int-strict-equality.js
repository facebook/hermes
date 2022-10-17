/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermes -O -dump-bytecode %s | %FileCheck --check-prefix=CHKBC %s

// Strick equality check on int32 and int32
function test_int_int(x, y) {
  x = x | 0;
  y = y | 0;
  if (x === y) {
    return x;
  } else {
    return undefined;
  }
}
//CHKBC: JStrictEqual

// Strick equality check on int32 and uint32
function test_int_uint(x, y) {
  x = x | 0;
  y = y >>> 0;
  if (x === y) {
    return x;
  } else {
    return undefined;
  }
}
//CHKBC: JStrictEqual

// Strick equality check on uint32 and uint32
function test_uint_uint(x, y) {
  x = x >>> 0;
  y = y >>> 0;
  if (x === y) {
    return x;
  } else {
    return undefined;
  }
}
//CHKBC: JStrictEqual

// Strick equality check on values that could be int
function test_could_be_int(func) {
  var x = func() * 100;
  var a = func() ? (x | 0) : undefined;
  var b = x >>> 0;
  if (a === b) {
    return x;
  } else {
    return undefined;
  }
}
//CHKBC: JStrictEqual

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [test_int_int, test_int_uint, test_uint_uint, test_could_be_int]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %test_int_int#0#1()#2 : undefined|number, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "test_int_int" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test_int_uint#0#1()#3 : undefined|number, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test_int_uint" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_uint_uint#0#1()#4 : undefined|number, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test_uint_uint" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_could_be_int#0#1()#5 : undefined|number, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "test_could_be_int" : string
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_int_int#0#1(x, y)#2 : undefined|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_int_int#0#1()#2}
// CHECK-NEXT:  %1 = AsInt32Inst %x
// CHECK-NEXT:  %2 = AsInt32Inst %y
// CHECK-NEXT:  %3 = BinaryOperatorInst '===', %1 : number, %2 : number
// CHECK-NEXT:  %4 = CondBranchInst %3 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst %1 : number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_int_uint#0#1(x, y)#3 : undefined|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_int_uint#0#1()#3}
// CHECK-NEXT:  %1 = AsInt32Inst %x
// CHECK-NEXT:  %2 = BinaryOperatorInst '>>>', %y, 0 : number
// CHECK-NEXT:  %3 = BinaryOperatorInst '===', %1 : number, %2 : number
// CHECK-NEXT:  %4 = CondBranchInst %3 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst %1 : number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_uint_uint#0#1(x, y)#4 : undefined|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_uint_uint#0#1()#4}
// CHECK-NEXT:  %1 = BinaryOperatorInst '>>>', %x, 0 : number
// CHECK-NEXT:  %2 = BinaryOperatorInst '>>>', %y, 0 : number
// CHECK-NEXT:  %3 = BinaryOperatorInst '===', %1 : number, %2 : number
// CHECK-NEXT:  %4 = CondBranchInst %3 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst %1 : number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_could_be_int#0#1(func)#5 : undefined|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_could_be_int#0#1()#5}
// CHECK-NEXT:  %1 = CallInst %func, undefined : undefined
// CHECK-NEXT:  %2 = BinaryOperatorInst '*', %1, 100 : number
// CHECK-NEXT:  %3 = CallInst %func, undefined : undefined
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = AsInt32Inst %2 : number
// CHECK-NEXT:  %6 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = PhiInst %5 : number, %BB1, undefined : undefined, %BB0
// CHECK-NEXT:  %8 = BinaryOperatorInst '>>>', %2 : number, 0 : number
// CHECK-NEXT:  %9 = BinaryOperatorInst '===', %7 : undefined|number, %8 : number
// CHECK-NEXT:  %10 = CondBranchInst %9 : boolean, %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = ReturnInst %2 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
