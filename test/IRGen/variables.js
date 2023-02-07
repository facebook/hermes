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

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "same_func_name" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "sink" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "level0" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %same_func_name()
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, globalObject : object, "same_func_name" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %sink()
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %level0()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "level0" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function same_func_name(same_param_name)
// CHECK-NEXT:frame = [same_param_name, same_func_name]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %same_param_name
// CHECK-NEXT:  %1 = StoreFrameInst %0, [same_param_name]
// CHECK-NEXT:  %2 = CreateFunctionInst %"same_func_name 1#"()
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [same_func_name]
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink(a, b, c)
// CHECK-NEXT:frame = [a, b, c]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadParamInst %b
// CHECK-NEXT:  %3 = StoreFrameInst %2, [b]
// CHECK-NEXT:  %4 = LoadParamInst %c
// CHECK-NEXT:  %5 = StoreFrameInst %4, [c]
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function level0(x)
// CHECK-NEXT:frame = [x, level1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = CreateFunctionInst %level1()
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [level1]
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 1#"(same_param_name)
// CHECK-NEXT:frame = [same_param_name, same_func_name]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %same_param_name
// CHECK-NEXT:  %1 = StoreFrameInst %0, [same_param_name]
// CHECK-NEXT:  %2 = CreateFunctionInst %"same_func_name 2#"()
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [same_func_name]
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function level1(y)
// CHECK-NEXT:frame = [y, level2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %y
// CHECK-NEXT:  %1 = StoreFrameInst %0, [y]
// CHECK-NEXT:  %2 = CreateFunctionInst %level2()
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [level2]
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 2#"(same_param_name)
// CHECK-NEXT:frame = [same_param_name]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %same_param_name
// CHECK-NEXT:  %1 = StoreFrameInst %0, [same_param_name]
// CHECK-NEXT:  %2 = LoadFrameInst [same_param_name]
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function level2(z)
// CHECK-NEXT:frame = [z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %z
// CHECK-NEXT:  %1 = StoreFrameInst %0, [z]
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %3 = LoadFrameInst [x@level0]
// CHECK-NEXT:  %4 = LoadFrameInst [y@level1]
// CHECK-NEXT:  %5 = LoadFrameInst [z]
// CHECK-NEXT:  %6 = CallInst %2, empty, empty, undefined : undefined, %3, %4, %5
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
