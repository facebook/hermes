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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [same_func_name, sink, level0]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %same_func_name#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "same_func_name" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %sink#0#1()#5, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "sink" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %level0#0#1()#6, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "level0" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = LoadStackInst %7
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function same_func_name#0#1(same_param_name)#2
// CHECK-NEXT:frame = [same_param_name#2, same_func_name#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{same_func_name#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %same_param_name, [same_param_name#2], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %"same_func_name 1#"#1#2()#3, %0
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [same_func_name#2], %0
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 1#"#1#2(same_param_name)#3
// CHECK-NEXT:frame = [same_param_name#3, same_func_name#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"same_func_name 1#"#1#2()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %same_param_name, [same_param_name#3], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %"same_func_name 2#"#2#3()#4, %0
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [same_func_name#3], %0
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "same_func_name 2#"#2#3(same_param_name)#4
// CHECK-NEXT:frame = [same_param_name#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"same_func_name 2#"#2#3()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %same_param_name, [same_param_name#4], %0
// CHECK-NEXT:  %2 = LoadFrameInst [same_param_name#4], %0
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function sink#0#1(a, b, c)#5
// CHECK-NEXT:frame = [a#5, b#5, c#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{sink#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#5], %0
// CHECK-NEXT:  %3 = StoreFrameInst %c, [c#5], %0
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function level0#0#1(x)#6
// CHECK-NEXT:frame = [x#6, level1#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{level0#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#6], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %level1#1#6()#7, %0
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [level1#6], %0
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function level1#1#6(y)#7
// CHECK-NEXT:frame = [y#7, level2#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{level1#1#6()#7}
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y#7], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %level2#6#7()#8, %0
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [level2#7], %0
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function level2#6#7(z)#8
// CHECK-NEXT:frame = [z#8]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{level2#6#7()#8}
// CHECK-NEXT:  %1 = StoreFrameInst %z, [z#8], %0
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "sink" : string
// CHECK-NEXT:  %3 = LoadFrameInst [x#6@level0], %0
// CHECK-NEXT:  %4 = LoadFrameInst [y#7@level1], %0
// CHECK-NEXT:  %5 = LoadFrameInst [z#8], %0
// CHECK-NEXT:  %6 = CallInst %2, undefined : undefined, %3, %4, %5
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
