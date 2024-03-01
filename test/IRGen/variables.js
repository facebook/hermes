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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "same_func_name": string
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:       DeclareGlobalVarInst "level0": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %same_func_name(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "same_func_name": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %sink(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "sink": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %level0(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "level0": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function same_func_name(same_param_name: any): any
// CHECK-NEXT:frame = [same_param_name: any, same_func_name: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %same_func_name(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %same_param_name: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [same_param_name]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [same_func_name]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %"same_func_name 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [same_func_name]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function sink(a: any, b: any, c: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %sink(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [b]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [c]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function level0(x: any): any
// CHECK-NEXT:frame = [x: any, level1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %level0(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [level1]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %level1(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [level1]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 1#"(same_param_name: any): any
// CHECK-NEXT:frame = [same_param_name: any, same_func_name: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %same_func_name(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"same_func_name 1#"(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %same_param_name: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [same_param_name]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [same_func_name]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %"same_func_name 2#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [same_func_name]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function level1(y: any): any
// CHECK-NEXT:frame = [y: any, level2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %level0(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %level1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [y]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [level2]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %level2(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [level2]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 2#"(same_param_name: any): any
// CHECK-NEXT:frame = [same_param_name: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %"same_func_name 1#"(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"same_func_name 2#"(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %same_param_name: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [same_param_name]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [same_param_name]: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function level2(z: any): any
// CHECK-NEXT:frame = [z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %level1(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %level2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [z]: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %5 = ResolveScopeInst (:environment) %level0(): any, %1: environment
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %5: environment, [x@level0]: any
// CHECK-NEXT:  %7 = ResolveScopeInst (:environment) %level1(): any, %1: environment
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %7: environment, [y@level1]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [z]: any
// CHECK-NEXT:  %10 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %6: any, %8: any, %9: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
