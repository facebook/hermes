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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "test0": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test1": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test2": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "test_hoisting_of_func_expr": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %test0(): any
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: object, globalObject: object, "test0": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %test1(): any
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: object, globalObject: object, "test1": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %test2(): any
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8: object, globalObject: object, "test2": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %test_hoisting_of_func_expr(): any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: object, globalObject: object, "test_hoisting_of_func_expr": string
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %13 = StoreStackInst undefined: undefined, %12: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %12: any
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function test0(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:  %1 = CallInst (:any) %0: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test1(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %" 1#"(): any
// CHECK-NEXT:  %3 = CallInst (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test2(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %" 2#"(): any
// CHECK-NEXT:  %3 = CallInst (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_hoisting_of_func_expr(): any
// CHECK-NEXT:frame = [some_local_name: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %some_local_name(): any
// CHECK-NEXT:  %1 = StoreFrameInst %0: object, [some_local_name]: any
// CHECK-NEXT:  %2 = CallInst (:any) %0: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function " 1#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [x@test1]: any
// CHECK-NEXT:  %1 = ReturnInst %0: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function " 2#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %" 3#"(): any
// CHECK-NEXT:  %1 = CallInst (:any) %0: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function some_local_name(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:any) [some_local_name@test_hoisting_of_func_expr]: any
// CHECK-NEXT:  %1 = ReturnInst %0: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function " 3#"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 2: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
