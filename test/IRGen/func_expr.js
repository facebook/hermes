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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
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

// CHECK:scope %VS1 []

// CHECK:function test0(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %""(): functionCode
// CHECK-NEXT:  %3 = CallInst (:any) %2: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any]

// CHECK:function test1(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.x]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %5 = CallInst (:any) %4: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [x: any]

// CHECK:function test2(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.x]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %" 2#"(): functionCode
// CHECK-NEXT:  %5 = CallInst (:any) %4: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [some_local_name: any]

// CHECK:function test_hoisting_of_func_expr(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %some_local_name(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS4.some_local_name]: any
// CHECK-NEXT:  %4 = CallInst (:any) %2: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:function ""(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:scope %VS6 []

// CHECK:function " 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [%VS2.x]: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:scope %VS7 []

// CHECK:function " 2#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %" 3#"(): functionCode
// CHECK-NEXT:  %3 = CallInst (:any) %2: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS8 []

// CHECK:function some_local_name(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS8: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [%VS4.some_local_name]: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:scope %VS9 []

// CHECK:function " 3#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS7: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS9: any, %0: environment
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end
