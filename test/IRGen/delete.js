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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:       DeclareGlobalVarInst "x": string
// CHECK-NEXT:       DeclareGlobalVarInst "delete_parameter": string
// CHECK-NEXT:       DeclareGlobalVarInst "delete_literal": string
// CHECK-NEXT:       DeclareGlobalVarInst "delete_variable": string
// CHECK-NEXT:       DeclareGlobalVarInst "delete_expr": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %sink(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "sink": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %delete_parameter(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "delete_parameter": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %delete_literal(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "delete_literal": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %delete_variable(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "delete_variable": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %delete_expr(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "delete_expr": string
// CHECK-NEXT:  %17 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %17: any
// CHECK-NEXT:  %19 = LoadStackInst (:any) %17: any
// CHECK-NEXT:        ReturnInst %19: any
// CHECK-NEXT:function_end

// CHECK:function sink(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %sink(): any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function delete_parameter(p: any): any
// CHECK-NEXT:frame = [p: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %delete_parameter(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [p]: any
// CHECK-NEXT:       ReturnInst false: boolean
// CHECK-NEXT:function_end

// CHECK:function delete_literal(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %delete_literal(): any, %0: environment
// CHECK-NEXT:       ReturnInst true: boolean
// CHECK-NEXT:function_end

// CHECK:function delete_variable(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %delete_variable(): any, %0: environment
// CHECK-NEXT:  %2 = DeletePropertyLooseInst (:any) globalObject: object, "x": string
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function delete_expr(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %delete_expr(): any, %0: environment
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst true: boolean
// CHECK-NEXT:function_end
