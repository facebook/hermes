/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

//CHECK: function test0()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = CreateFunctionInst %""()
//CHECK:     %1 = CallInst %0 : closure, undefined : undefined
//CHECK:     %2 = ReturnInst undefined : undefined

//CHECK: function ""()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = ReturnInst 1 : number
//CHECK:   %BB1:
//CHECK:     %1 = ReturnInst undefined : undefined

// Simple function expr.
function test0() {
  (function() { return 1;})();
}

//CHECK: function test1(x)
//CHECK: frame = [x]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = CreateFunctionInst %" 1#"()
//CHECK:     %2 = CallInst %1 : closure, undefined : undefined
//CHECK:     %3 = ReturnInst undefined : undefined

//CHECK: function " 1#"()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = LoadFrameInst [x@test1]
//CHECK:     %1 = ReturnInst %0
//CHECK:   %BB1:
//CHECK:     %2 = ReturnInst undefined : undefined

// Capture a parameter.
function test1(x) {
  (function() { return x;})();
}

//CHECK: function test2(x)
//CHECK: frame = [x]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %x, [x]
//CHECK:     %1 = CreateFunctionInst %" 2#"()
//CHECK:     %2 = CallInst %1 : closure, undefined : undefined
//CHECK:     %3 = ReturnInst undefined : undefined

//CHECK: function " 2#"()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = CreateFunctionInst %" 3#"()
//CHECK:     %1 = CallInst %0 : closure, undefined : undefined
//CHECK:     %2 = ReturnInst undefined : undefined
//CHECK: function " 3#"()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = ReturnInst 2 : number
//CHECK:   %BB1:
//CHECK:     %1 = ReturnInst undefined : undefined

// Nested function expr.
function test2(x) {
  (function() {
    (function() { return 2; })();
  })();
}

//CHECK-LABEL:function test_hoisting_of_func_expr()
//CHECK-NEXT:frame = [?anon_0_closure]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = CreateFunctionInst %some_local_name()
//CHECK-NEXT:    %1 = StoreFrameInst %0 : closure, [?anon_0_closure]
//CHECK-NEXT:    %2 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:    %3 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
//CHECK-LABEL:function some_local_name()
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = LoadFrameInst [?anon_0_closure@test_hoisting_of_func_expr]
//CHECK-NEXT:    %1 = ReturnInst %0
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %2 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_hoisting_of_func_expr() {
  (function some_local_name() {
   return some_local_name;
   } () );
 }
