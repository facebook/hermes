/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheck --match-full-lines %s
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
//CHECK-LABEL: function test_int_int(x, y) : undefined|number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = AsInt32Inst %x
//CHECK-NEXT:   %1 = AsInt32Inst %y
//CHECK-NEXT:   %2 = BinaryOperatorInst '===', %0 : number, %1 : number
//CHECK-NEXT:   %3 = CondBranchInst %2 : boolean, %BB1, %BB2
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %4 = ReturnInst %0 : number
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %5 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

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
//CHECK-LABEL: function test_int_uint(x, y) : undefined|number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = AsInt32Inst %x
//CHECK-NEXT:   %1 = BinaryOperatorInst '>>>', %y, 0 : number
//CHECK-NEXT:   %2 = BinaryOperatorInst '===', %0 : number, %1 : number
//CHECK-NEXT:   %3 = CondBranchInst %2 : boolean, %BB1, %BB2
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %4 = ReturnInst %0 : number
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %5 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

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
//CHECK-LABEL: function test_uint_uint(x, y) : undefined|number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = BinaryOperatorInst '>>>', %x, 0 : number
//CHECK-NEXT:   %1 = BinaryOperatorInst '>>>', %y, 0 : number
//CHECK-NEXT:   %2 = BinaryOperatorInst '===', %0 : number, %1 : number
//CHECK-NEXT:   %3 = CondBranchInst %2 : boolean, %BB1, %BB2
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %4 = ReturnInst %0 : number
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %5 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end

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
//CHECK-LABEL: function test_could_be_int(func) : undefined|number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = CallInst %func, undefined : undefined
//CHECK-NEXT:   %1 = BinaryOperatorInst '*', %0, 100 : number
//CHECK-NEXT:   %2 = CallInst %func, undefined : undefined
//CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %4 = AsInt32Inst %1 : number
//CHECK-NEXT:   %5 = BranchInst %BB2
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %6 = PhiInst %4 : number, %BB1, undefined : undefined, %BB0
//CHECK-NEXT:   %7 = BinaryOperatorInst '>>>', %1 : number, 0 : number
//CHECK-NEXT:   %8 = BinaryOperatorInst '===', %6 : undefined|number, %7 : number
//CHECK-NEXT:   %9 = CondBranchInst %8 : boolean, %BB3, %BB4
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %10 = ReturnInst %1 : number
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %11 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
