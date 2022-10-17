/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O -fno-inline | %FileCheckOrRegen %s

"use strict";

function main(p) {
  function foo(x, y) { return x + y; }
  function bar(x, y) { return x + y; }

  // capture bar.
  p.p = bar;

  return foo(1,2) + bar (1,2)
}

function return_types(p) {
  function builder() { return k * 1 }

  return builder() + builder()
}

function test_unused_and_duplicate_params() {
  function foo2(a, b, c, d) {
    return a + b + c
  }

  function bar1(e) {
    foo2(e ,2, 1, 4, 5, "")
  }

  function bar2(e) {
    foo2(e, 2, 3 ,4 ,5)
  }
  return [bar1, bar2]
}

// Usage of rest arguments should disable signature optimization.
function test_rest_arguments() {
  function baz(...rest) { return rest; }
  return baz(100);
}

function test_generator() {
  function* gen(x) { return x; }
  return gen(1);
}

function test_async() {
  async function asyncFn(x) { return x; }
  return asyncFn(1);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1 : string
// CHECK-NEXT:frame = [], globals = [main, return_types, test_unused_and_duplicate_params, test_rest_arguments, test_generator, test_async]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %main#0#1()#2 : string|number, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %return_types#0#1()#5 : number, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "return_types" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_unused_and_duplicate_params#0#1()#7 : object, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test_unused_and_duplicate_params" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_rest_arguments#0#1()#11, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "test_rest_arguments" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %test_generator#0#1()#13 : object, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "test_generator" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %test_async#0#1()#16, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "test_async" : string
// CHECK-NEXT:  %13 = ReturnInst "use strict" : string
// CHECK-NEXT:function_end

// CHECK:function main#0#1(p)#2 : string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{main#0#1()#2}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#1#2()#3 : number, %0
// CHECK-NEXT:  %2 = CreateFunctionInst %bar#1#2()#4 : string|number|bigint, %0
// CHECK-NEXT:  %3 = StorePropertyInst %2 : closure, %p, "p" : string
// CHECK-NEXT:  %4 = CallInst %1 : closure, undefined : undefined, 1 : number, 2 : number
// CHECK-NEXT:  %5 = CallInst %2 : closure, undefined : undefined, 1 : number, 2 : number
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %4 : number, %5 : string|number|bigint
// CHECK-NEXT:  %7 = ReturnInst %6 : string|number
// CHECK-NEXT:function_end

// CHECK:function foo#1#2(x : number, y : number)#3 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#1#2()#3}
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : number
// CHECK-NEXT:function_end

// CHECK:function bar#1#2(x, y)#4 : string|number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bar#1#2()#4}
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %2 = ReturnInst %1 : string|number|bigint
// CHECK-NEXT:function_end

// CHECK:function return_types#0#1(p)#5 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{return_types#0#1()#5}
// CHECK-NEXT:  %1 = CreateFunctionInst %builder#1#5()#6 : number, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %3 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %4 = BinaryOperatorInst '+', %2 : number, %3 : number
// CHECK-NEXT:  %5 = ReturnInst %4 : number
// CHECK-NEXT:function_end

// CHECK:function builder#1#5()#6 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{builder#1#5()#6}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "k" : string
// CHECK-NEXT:  %2 = BinaryOperatorInst '*', %1, 1 : number
// CHECK-NEXT:  %3 = ReturnInst %2 : number
// CHECK-NEXT:function_end

// CHECK:function test_unused_and_duplicate_params#0#1()#7 : object
// CHECK-NEXT:frame = [foo2#7 : closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_unused_and_duplicate_params#0#1()#7}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo2#1#7()#8 : string|number, %0
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [foo2#7] : closure, %0
// CHECK-NEXT:  %3 = CreateFunctionInst %bar1#1#7()#9 : undefined, %0
// CHECK-NEXT:  %4 = CreateFunctionInst %bar2#1#7()#10 : undefined, %0
// CHECK-NEXT:  %5 = AllocArrayInst 2 : number
// CHECK-NEXT:  %6 = StoreOwnPropertyInst %3 : closure, %5 : object, 0 : number, true : boolean
// CHECK-NEXT:  %7 = StoreOwnPropertyInst %4 : closure, %5 : object, 1 : number, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %5 : object
// CHECK-NEXT:function_end

// CHECK:function foo2#1#7(a, b, c, d)#8 : string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo2#1#7()#8}
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', %a, 2 : number
// CHECK-NEXT:  %2 = BinaryOperatorInst '+', %1 : string|number, %c
// CHECK-NEXT:  %3 = ReturnInst %2 : string|number
// CHECK-NEXT:function_end

// CHECK:function bar1#1#7(e)#9 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bar1#1#7()#9}
// CHECK-NEXT:  %1 = LoadFrameInst [foo2#7@test_unused_and_duplicate_params] : closure, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, %e, 2 : number, 1 : number, undefined : undefined, undefined : undefined, undefined : undefined
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar2#1#7(e)#10 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bar2#1#7()#10}
// CHECK-NEXT:  %1 = LoadFrameInst [foo2#7@test_unused_and_duplicate_params] : closure, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, %e, 2 : number, 3 : number, undefined : undefined, undefined : undefined
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_rest_arguments#0#1()#11
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_rest_arguments#0#1()#11}
// CHECK-NEXT:  %1 = CreateFunctionInst %baz#1#11()#12, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, 100 : number
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:function_end

// CHECK:function baz#1#11()#12
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{baz#1#11()#12}
// CHECK-NEXT:  %1 = CallBuiltinInst [HermesBuiltin.copyRestArgs] : number, undefined : undefined, 0 : number
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:function_end

// CHECK:function test_generator#0#1()#13 : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_generator#0#1()#13}
// CHECK-NEXT:  %1 = CreateFunctionInst %gen#1#13()#14 : object, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, 1 : number
// CHECK-NEXT:  %3 = ReturnInst %2 : object
// CHECK-NEXT:function_end

// CHECK:function gen#1#13()#14 : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{gen#1#13()#14}
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_gen#13#14()#15, %0
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_gen#13#14(x)#15
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = CreateScopeInst %S{?anon_0_gen#13#14()#15}
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst %x
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst %3
// CHECK-NEXT:function_end

// CHECK:function test_async#0#1()#16
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_async#0#1()#16}
// CHECK-NEXT:  %1 = CreateFunctionInst %asyncFn#1#16()#17, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, 1 : number
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:function_end

// CHECK:function asyncFn#1#16()#17
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{asyncFn#1#16()#17}
// CHECK-NEXT:  %1 = CreateArgumentsInst
// CHECK-NEXT:  %2 = CreateFunctionInst %?anon_0_asyncFn#16#17()#18 : object, %0
// CHECK-NEXT:  %3 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %4 = CallInst %3 : closure, undefined : undefined, %2 : closure, %this, %1 : object
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_asyncFn#16#17()#18 : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{?anon_0_asyncFn#16#17()#18}
// CHECK-NEXT:  %1 = CreateGeneratorInst %?anon_0_?anon_0_asyncFn#17#18()#19, %0
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_asyncFn#17#18(x)#19
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = CreateScopeInst %S{?anon_0_?anon_0_asyncFn#17#18()#19}
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %3 = ResumeGeneratorInst %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = ReturnInst %x
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst %3
// CHECK-NEXT:function_end
