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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [x, sink, delete_parameter, delete_literal, delete_variable, delete_expr]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %sink#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %delete_parameter#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "delete_parameter" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %delete_literal#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "delete_literal" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %delete_variable#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "delete_variable" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %delete_expr#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "delete_expr" : string
// CHECK-NEXT:  %11 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
// CHECK-NEXT:  %13 = LoadStackInst %11
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:function_end

// CHECK:function sink#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{sink#0#1()#2}
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_parameter#0#1(p)#3
// CHECK-NEXT:frame = [p#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{delete_parameter#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %p, [p#3], %0
// CHECK-NEXT:  %2 = ReturnInst false : boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_literal#0#1()#4
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{delete_literal#0#1()#4}
// CHECK-NEXT:  %1 = ReturnInst true : boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_variable#0#1()#5
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{delete_variable#0#1()#5}
// CHECK-NEXT:  %1 = DeletePropertyInst globalObject : object, "x" : string
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_expr#0#1()#6
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{delete_expr#0#1()#6}
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined
// CHECK-NEXT:  %3 = ReturnInst true : boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
