/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O | %FileCheckOrRegen %s

function foo(dim) {
  var a = (dim == dim); // This creates a bool.
  var b = (dim == dim); // This creates a bool.
  var c = a + b;
  var d = a + b;
  return c * d;
}

// This is supposed to be a CSE across blocks, but LoadStoreOpts is
// not across basic blocks, we end up loading some values for the BinaryOperator.
function foo_with_cf(dim) {
  var d = 0;
  var a = (dim == dim);
  var b = (dim == dim);
  var c = a + b;
  if (a) {
    d = a + b;
  }
  return c * d;
}

// Make sure we are not merging the plus and minus together.
function check_operator_kind(i) {
  var x = i | 0;
  var y = i | 0;
  var t0 = x - y;
  var t1 = x + y;
  return t0 * t1;
}

function cse_this_instr() {
  print(this, this, this, this)
}

function cse_unary(a) {
    ++a;
    return (-a) * (-a);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [foo, foo_with_cf, check_operator_kind, cse_this_instr, cse_unary]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo() : number
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %foo_with_cf() : number
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "foo_with_cf" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %check_operator_kind() : number
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "check_operator_kind" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %cse_this_instr() : undefined
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "cse_this_instr" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %cse_unary() : number|bigint
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "cse_unary" : string
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo(dim) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryOperatorInst '==', %dim, %dim
// CHECK-NEXT:  %1 = BinaryOperatorInst '==', %dim, %dim
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', %0 : boolean, %1 : boolean
// CHECK-NEXT:  %3 = BinaryOperatorInst '*', %2 : number, %2 : number
// CHECK-NEXT:  %4 = ReturnInst %3 : number
// CHECK-NEXT:function_end

// CHECK:function foo_with_cf(dim) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryOperatorInst '==', %dim, %dim
// CHECK-NEXT:  %1 = BinaryOperatorInst '==', %dim, %dim
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', %0 : boolean, %1 : boolean
// CHECK-NEXT:  %3 = CondBranchInst %0 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = PhiInst %2 : number, %BB1, 0 : number, %BB0
// CHECK-NEXT:  %6 = BinaryOperatorInst '*', %2 : number, %5 : number
// CHECK-NEXT:  %7 = ReturnInst %6 : number
// CHECK-NEXT:function_end

// CHECK:function check_operator_kind(i) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AsInt32Inst %i
// CHECK-NEXT:  %1 = AsInt32Inst %i
// CHECK-NEXT:  %2 = BinaryOperatorInst '-', %0 : number, %1 : number
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', %0 : number, %1 : number
// CHECK-NEXT:  %4 = BinaryOperatorInst '*', %2 : number, %3 : number
// CHECK-NEXT:  %5 = ReturnInst %4 : number
// CHECK-NEXT:function_end

// CHECK:function cse_this_instr() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, %this, %this, %this, %this
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function cse_unary(a) : number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = UnaryOperatorInst '++', %a
// CHECK-NEXT:  %1 = UnaryOperatorInst '-', %0 : number|bigint
// CHECK-NEXT:  %2 = BinaryOperatorInst '*', %1 : number|bigint, %1 : number|bigint
// CHECK-NEXT:  %3 = ReturnInst %2 : number|bigint
// CHECK-NEXT:function_end
