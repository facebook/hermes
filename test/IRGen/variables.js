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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "same_func_name": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "sink": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "level0": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %same_func_name(): any
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3: object, globalObject: object, "same_func_name": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %sink(): any
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5: object, globalObject: object, "sink": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %level0(): any
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: object, globalObject: object, "level0": string
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %10 = StoreStackInst undefined: undefined, %9: any
// CHECK-NEXT:  %11 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %12 = ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function same_func_name(same_param_name: any): any
// CHECK-NEXT:frame = [same_param_name: any, same_func_name: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %same_param_name: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [same_param_name]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [same_func_name]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %"same_func_name 1#"(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: object, [same_func_name]: any
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function sink(a: any, b: any, c: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [c]: any
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function level0(x: any): any
// CHECK-NEXT:frame = [x: any, level1: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [level1]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %level1(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: object, [level1]: any
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 1#"(same_param_name: any): any
// CHECK-NEXT:frame = [same_param_name: any, same_func_name: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %same_param_name: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [same_param_name]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [same_func_name]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %"same_func_name 2#"(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: object, [same_func_name]: any
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function level1(y: any): any
// CHECK-NEXT:frame = [y: any, level2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [y]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [level2]: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %level2(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: object, [level2]: any
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 2#"(same_param_name: any): any
// CHECK-NEXT:frame = [same_param_name: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %same_param_name: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [same_param_name]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [same_param_name]: any
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function level2(z: any): any
// CHECK-NEXT:frame = [z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [z]: any
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [x@level0]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [y@level1]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %6 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: any, %4: any, %5: any
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
