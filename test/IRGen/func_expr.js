/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [test0, test1, test2, test_hoisting_of_func_expr]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %test0#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "test0" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %test1#0#1()#4, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "test1" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test2#0#1()#6, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_hoisting_of_func_expr#0#1()#9, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "test_hoisting_of_func_expr" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function test0#0#1()#2
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test0#0#1()#2}
// CHECK-NEXT:  %1 = CreateFunctionInst %""#1#2()#3, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function ""#1#2()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{""#1#2()#3}
// CHECK-NEXT:  %1 = ReturnInst 1 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test1#0#1(x)#4
// CHECK-NEXT:frame = [x#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test1#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#4], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %" 1#"#1#4()#5, %0
// CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function " 1#"#1#4()#5
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{" 1#"#1#4()#5}
// CHECK-NEXT:  %1 = LoadFrameInst [x#4@test1], %0
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2#0#1(x)#6
// CHECK-NEXT:frame = [x#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test2#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#6], %0
// CHECK-NEXT:  %2 = CreateFunctionInst %" 2#"#1#6()#7, %0
// CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function " 2#"#1#6()#7
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{" 2#"#1#6()#7}
// CHECK-NEXT:  %1 = CreateFunctionInst %" 3#"#6#7()#8, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function " 3#"#6#7()#8
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{" 3#"#6#7()#8}
// CHECK-NEXT:  %1 = ReturnInst 2 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_hoisting_of_func_expr#0#1()#9
// CHECK-NEXT:frame = [?anon_0_closure#9]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_hoisting_of_func_expr#0#1()#9}
// CHECK-NEXT:  %1 = CreateFunctionInst %some_local_name#1#9()#10, %0
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [?anon_0_closure#9], %0
// CHECK-NEXT:  %3 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function some_local_name#1#9()#10
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{some_local_name#1#9()#10}
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_0_closure#9@test_hoisting_of_func_expr], %0
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
