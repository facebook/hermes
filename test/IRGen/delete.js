/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "sink" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "x" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "delete_parameter" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "delete_literal" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "delete_variable" : string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "delete_expr" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %sink()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %delete_parameter()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "delete_parameter" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %delete_literal()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "delete_literal" : string
// CHECK-NEXT:  %12 = CreateFunctionInst %delete_variable()
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "delete_variable" : string
// CHECK-NEXT:  %14 = CreateFunctionInst %delete_expr()
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14 : closure, globalObject : object, "delete_expr" : string
// CHECK-NEXT:  %16 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %17 = StoreStackInst undefined : undefined, %16
// CHECK-NEXT:  %18 = LoadStackInst %16
// CHECK-NEXT:  %19 = ReturnInst %18
// CHECK-NEXT:function_end

// CHECK:function sink()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_parameter(p)
// CHECK-NEXT:frame = [p]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %p
// CHECK-NEXT:  %1 = StoreFrameInst %0, [p]
// CHECK-NEXT:  %2 = ReturnInst false : boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
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
// CHECK-NEXT:  %0 = DeletePropertyLooseInst globalObject : object, "x" : string
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function delete_expr()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %1 = CallInst %0, empty, empty, undefined : undefined
// CHECK-NEXT:  %2 = ReturnInst true : boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
