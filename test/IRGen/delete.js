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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "sink": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "delete_parameter": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "delete_literal": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "delete_variable": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "delete_expr": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %sink(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: object, globalObject: object, "sink": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %delete_parameter(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: object, globalObject: object, "delete_parameter": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %delete_literal(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: object, globalObject: object, "delete_literal": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %delete_variable(): any
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12: object, globalObject: object, "delete_variable": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %delete_expr(): any
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14: object, globalObject: object, "delete_expr": string
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %17 = StoreStackInst undefined: undefined, %16: any
// CHECK-NEXT:  %18 = LoadStackInst (:any) %16: any
// CHECK-NEXT:  %19 = ReturnInst %18: any
// CHECK-NEXT:function_end

// CHECK:function sink(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function delete_parameter(p: any): any
// CHECK-NEXT:frame = [p: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [p]: any
// CHECK-NEXT:  %2 = ReturnInst false: boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function delete_literal(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst true: boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function delete_variable(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeletePropertyLooseInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %1 = ReturnInst %0: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function delete_expr(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst true: boolean
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = UnreachableInst
// CHECK-NEXT:function_end
