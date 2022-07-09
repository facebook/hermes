/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O | %FileCheck %s

//CHECK-LABEL: function foo(dim)
//CHECK-NEXT:frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT: [[RET0:%.*]] = BinaryOperatorInst '==', %dim, %dim
//CHECK-NEXT: [[RET1:%.*]] = BinaryOperatorInst '==', %dim, %dim
//CHECK-NEXT: [[RET2:%.*]] = BinaryOperatorInst '+', [[RET0]] : boolean, [[RET1]] : boolean
//CHECK-NEXT: [[RET3:%.*]] = BinaryOperatorInst '*', [[RET2]] : number, [[RET2]] : number
//CHECK-NEXT: [[RET4:%.*]] = ReturnInst [[RET3]] : number
//CHECK-NEXT:function_end
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
//CHECK-LABEL:function check_operator_kind(i)
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = AsInt32Inst %i
//CHECK-NEXT:    %1 = AsInt32Inst %i
//CHECK-NEXT:    %2 = BinaryOperatorInst '-', %0 : number, %1 : number
//CHECK-NEXT:    %3 = BinaryOperatorInst '+', %0 : number, %1 : number
//CHECK-NEXT:    %4 = BinaryOperatorInst '*', %2 : number, %3 : number
//CHECK-NEXT:    %5 = ReturnInst %4 : number
//CHECK-NEXT:function_end
function check_operator_kind(i) {
  var x = i | 0;
  var y = i | 0;
  var t0 = x - y;
  var t1 = x + y;
  return t0 * t1;
}

//CHECK-LABEL:function cse_this_instr()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, %this, %this, %this, %this
//CHECK-NEXT:  %2 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function cse_this_instr() {
  print(this, this, this, this)
}

//CHECK-LABEL:function cse_unary(a)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = UnaryOperatorInst '++', %a
//CHECK-NEXT:  %1 = UnaryOperatorInst '-', %0 : number|bigint
//CHECK-NEXT:  %2 = BinaryOperatorInst '*', %1 : number|bigint, %1 : number|bigint
//CHECK-NEXT:  %3 = ReturnInst %2 : number|bigint
//CHECK-NEXT:function_end
function cse_unary(a) {
    ++a;
    return (-a) * (-a);
}
