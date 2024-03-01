/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

// Simple function expr.
function test0() {
  (function() { return 1;})();
}

// Capture a parameter.
function test1(x) {
  (function() { return x;})();
}

// Nested function expr.
function test2(x) {
  (function() {
    (function() { return 2; })();
  })();
}

function test_hoisting_of_func_expr() {
  (function some_local_name() {
   return some_local_name;
   } () );
 }

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "test0": string
// CHECK-NEXT:       DeclareGlobalVarInst "test1": string
// CHECK-NEXT:       DeclareGlobalVarInst "test2": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_hoisting_of_func_expr": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %test0(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "test0": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %test1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "test1": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %test2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "test2": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %test_hoisting_of_func_expr(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "test_hoisting_of_func_expr": string
// CHECK-NEXT:  %13 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %13: any
// CHECK-NEXT:  %15 = LoadStackInst (:any) %13: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:function_end

// CHECK:function test0(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test0(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:  %3 = CallInst (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test1(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %5 = CallInst (:any) %4: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test2(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %" 2#"(): functionCode
// CHECK-NEXT:  %5 = CallInst (:any) %4: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_hoisting_of_func_expr(): any
// CHECK-NEXT:frame = [some_local_name: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_hoisting_of_func_expr(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %some_local_name(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [some_local_name]: any
// CHECK-NEXT:  %4 = CallInst (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test0(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function " 1#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test1(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %" 1#"(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %test1(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [x@test1]: any
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:function " 2#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test2(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %" 2#"(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %" 3#"(): functionCode
// CHECK-NEXT:  %3 = CallInst (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function some_local_name(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %test_hoisting_of_func_expr(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %some_local_name(): any, %0: environment
// CHECK-NEXT:  %2 = ResolveScopeInst (:environment) %test_hoisting_of_func_expr(): any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %2: environment, [some_local_name@test_hoisting_of_func_expr]: any
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:function " 3#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %" 2#"(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %" 3#"(): any, %0: environment
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end
