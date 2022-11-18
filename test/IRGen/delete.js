/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function sink() {}
var x;

function delete_parameter(p) {
  return (delete p);
}

function delete_literal() {
  return (delete 4);
}

function delete_variable() {
  return (delete x);
}

function delete_expr() {
  return (delete sink());
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [x, sink, delete_parameter, delete_literal, delete_variable, delete_expr]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %sink()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %delete_parameter()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "delete_parameter" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %delete_literal()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "delete_literal" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %delete_variable()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "delete_variable" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %delete_expr()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "delete_expr" : string
// CHECK-NEXT:  %10 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %10
// CHECK-NEXT:  %12 = LoadStackInst %10
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:function_end

// CHECK:function sink()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_parameter(p)
// CHECK-NEXT:frame = [p]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %p, [p]
// CHECK-NEXT:  %1 = ReturnInst false : boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_literal()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst true : boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_variable()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeletePropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_expr()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined
// CHECK-NEXT:  %2 = ReturnInst true : boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
