/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function same_func_name(same_param_name) {
  function same_func_name(same_param_name) {
     function same_func_name(same_param_name) {
      return same_param_name;
    }
  }
}

function sink(a,b,c) {}

function level0(x) {
  function level1(y) {
    function level2(z) {
      sink(x,y,z)
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "same_func_name": string
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:       DeclareGlobalVarInst "level0": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %VS0: any, %same_func_name(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "same_func_name": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %VS0: any, %sink(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "sink": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %VS0: any, %level0(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "level0": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [same_param_name: any, same_func_name: any]

// CHECK:function same_func_name(same_param_name: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %same_param_name: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.same_param_name]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %VS1: any, %"same_func_name 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS1.same_func_name]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [a: any, b: any, c: any]

// CHECK:function sink(a: any, b: any, c: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS2.b]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [%VS2.c]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [x: any, level1: any]

// CHECK:function level0(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.x]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %VS3: any, %level1(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS3.level1]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [same_param_name: any, same_func_name: any]

// CHECK:function "same_func_name 1#"(same_param_name: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %same_param_name: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS4.same_param_name]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %VS4: any, %"same_func_name 2#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS4.same_func_name]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [y: any, level2: any]

// CHECK:function level1(y: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS3: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS5.y]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %VS5: any, %level2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS5.level2]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [same_param_name: any]

// CHECK:function "same_func_name 2#"(same_param_name: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS4: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %same_param_name: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS6.same_param_name]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS6.same_param_name]: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [z: any]

// CHECK:function level2(z: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS5: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS7.z]: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %5 = ResolveScopeInst (:environment) %VS3: any, %VS5: any, %0: environment
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %5: environment, [%VS3.x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %0: environment, [%VS5.y]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS7.z]: any
// CHECK-NEXT:  %9 = CallInst (:any) %4: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %6: any, %7: any, %8: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
