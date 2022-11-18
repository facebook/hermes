/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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
// CHECK-NEXT:frame = [], globals = [same_func_name, sink, level0]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %same_func_name()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "same_func_name" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %sink()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %level0()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "level0" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadStackInst %6
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function same_func_name(same_param_name)
// CHECK-NEXT:frame = [same_func_name, same_param_name]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %same_param_name, [same_param_name]
// CHECK-NEXT:  %1 = CreateFunctionInst %"same_func_name 1#"()
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [same_func_name]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 1#"(same_param_name)
// CHECK-NEXT:frame = [same_func_name, same_param_name]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %same_param_name, [same_param_name]
// CHECK-NEXT:  %1 = CreateFunctionInst %"same_func_name 2#"()
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [same_func_name]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 2#"(same_param_name)
// CHECK-NEXT:frame = [same_param_name]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %same_param_name, [same_param_name]
// CHECK-NEXT:  %1 = LoadFrameInst [same_param_name]
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink(a, b, c)
// CHECK-NEXT:frame = [a, b, c]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = StoreFrameInst %b, [b]
// CHECK-NEXT:  %2 = StoreFrameInst %c, [c]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function level0(x)
// CHECK-NEXT:frame = [level1, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = CreateFunctionInst %level1()
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [level1]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function level1(y)
// CHECK-NEXT:frame = [level2, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %1 = CreateFunctionInst %level2()
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [level2]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function level2(z)
// CHECK-NEXT:frame = [z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %z, [z]
// CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %2 = LoadFrameInst [x@level0]
// CHECK-NEXT:  %3 = LoadFrameInst [y@level1]
// CHECK-NEXT:  %4 = LoadFrameInst [z]
// CHECK-NEXT:  %5 = CallInst %1, undefined : undefined, %2, %3, %4
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
