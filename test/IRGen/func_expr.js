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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [test0, test1, test2, test_hoisting_of_func_expr]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %test0()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "test0" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %test1()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "test1" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test2()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "test2" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %test_hoisting_of_func_expr()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "test_hoisting_of_func_expr" : string
// CHECK-NEXT:  %8 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %9 = StoreStackInst undefined : undefined, %8
// CHECK-NEXT:  %10 = LoadStackInst %8
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:function_end

// CHECK:function test0()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %""()
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function ""()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test1(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = CreateFunctionInst %" 1#"()
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function " 1#"()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [x@test1]
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test2(x)
// CHECK-NEXT:frame = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = CreateFunctionInst %" 2#"()
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function " 2#"()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %" 3#"()
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function " 3#"()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 2 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_hoisting_of_func_expr()
// CHECK-NEXT:frame = [?anon_0_closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %some_local_name()
// CHECK-NEXT:  %1 = StoreFrameInst %0 : closure, [?anon_0_closure]
// CHECK-NEXT:  %2 = CallInst %0 : closure, undefined : undefined
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function some_local_name()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [?anon_0_closure@test_hoisting_of_func_expr]
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
