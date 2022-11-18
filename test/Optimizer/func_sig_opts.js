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

// CHECK:function global() : string
// CHECK-NEXT:frame = [], globals = [main, return_types, test_unused_and_duplicate_params, test_rest_arguments, test_generator, test_async]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %main() : string|number
// CHECK-NEXT:  %1 = StorePropertyStrictInst %0 : closure, globalObject : object, "main" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %return_types() : number
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2 : closure, globalObject : object, "return_types" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test_unused_and_duplicate_params() : object
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4 : closure, globalObject : object, "test_unused_and_duplicate_params" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %test_rest_arguments()
// CHECK-NEXT:  %7 = StorePropertyStrictInst %6 : closure, globalObject : object, "test_rest_arguments" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %test_generator() : object
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8 : closure, globalObject : object, "test_generator" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %test_async()
// CHECK-NEXT:  %11 = StorePropertyStrictInst %10 : closure, globalObject : object, "test_async" : string
// CHECK-NEXT:  %12 = ReturnInst "use strict" : string
// CHECK-NEXT:function_end

// CHECK:function main(p) : string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo() : number
// CHECK-NEXT:  %1 = CreateFunctionInst %bar() : string|number|bigint
// CHECK-NEXT:  %2 = StorePropertyStrictInst %1 : closure, %p, "p" : string
// CHECK-NEXT:  %3 = CallInst %0 : closure, undefined : undefined, 1 : number, 2 : number
// CHECK-NEXT:  %4 = CallInst %1 : closure, undefined : undefined, 1 : number, 2 : number
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %3 : number, %4 : string|number|bigint
// CHECK-NEXT:  %6 = ReturnInst %5 : string|number
// CHECK-NEXT:function_end

// CHECK:function foo(x : number, y : number) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryOperatorInst '+', 1 : number, 2 : number
// CHECK-NEXT:  %1 = ReturnInst %0 : number
// CHECK-NEXT:function_end

// CHECK:function bar(x, y) : string|number|bigint
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryOperatorInst '+', %x, %y
// CHECK-NEXT:  %1 = ReturnInst %0 : string|number|bigint
// CHECK-NEXT:function_end

// CHECK:function return_types(p) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %builder() : number
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined
// CHECK-NEXT:  %2 = CallInst %0 : closure, undefined : undefined
// CHECK-NEXT:  %3 = BinaryOperatorInst '+', %1 : number, %2 : number
// CHECK-NEXT:  %4 = ReturnInst %3 : number
// CHECK-NEXT:function_end

// CHECK:function builder() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "k" : string
// CHECK-NEXT:  %1 = BinaryOperatorInst '*', %0, 1 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : number
// CHECK-NEXT:function_end

// CHECK:function test_unused_and_duplicate_params() : object
// CHECK-NEXT:frame = [foo2 : closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo2() : string|number
// CHECK-NEXT:  %1 = StoreFrameInst %0 : closure, [foo2] : closure
// CHECK-NEXT:  %2 = CreateFunctionInst %bar1() : undefined
// CHECK-NEXT:  %3 = CreateFunctionInst %bar2() : undefined
// CHECK-NEXT:  %4 = AllocArrayInst 2 : number
// CHECK-NEXT:  %5 = StoreOwnPropertyInst %2 : closure, %4 : object, 0 : number, true : boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst %3 : closure, %4 : object, 1 : number, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:function_end

// CHECK:function foo2(a, b, c, d) : string|number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryOperatorInst '+', %a, 2 : number
// CHECK-NEXT:  %1 = BinaryOperatorInst '+', %0 : string|number, %c
// CHECK-NEXT:  %2 = ReturnInst %1 : string|number
// CHECK-NEXT:function_end

// CHECK:function bar1(e) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [foo2@test_unused_and_duplicate_params] : closure
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, %e, 2 : number, 1 : number, undefined : undefined, undefined : undefined, undefined : undefined
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar2(e) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [foo2@test_unused_and_duplicate_params] : closure
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, %e, 2 : number, 3 : number, undefined : undefined, undefined : undefined
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_rest_arguments()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %baz()
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 100 : number
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:function_end

// CHECK:function baz()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.copyRestArgs] : number, undefined : undefined, 0 : number
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:function_end

// CHECK:function test_generator() : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %gen() : object
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 1 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function gen() : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_gen()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_gen(x)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst %x
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst %2
// CHECK-NEXT:function_end

// CHECK:function test_async()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %asyncFn()
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 1 : number
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:function_end

// CHECK:function asyncFn()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsInst
// CHECK-NEXT:  %1 = CreateFunctionInst %?anon_0_asyncFn() : object
// CHECK-NEXT:  %2 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
// CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined, %1 : closure, %this, %0 : object
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_asyncFn() : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_?anon_0_asyncFn()
// CHECK-NEXT:  %1 = ReturnInst %0 : object
// CHECK-NEXT:function_end

// CHECK:function ?anon_0_?anon_0_asyncFn(x)
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StartGeneratorInst
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
// CHECK-NEXT:  %2 = ResumeGeneratorInst %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = ReturnInst %x
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = ReturnInst %2
// CHECK-NEXT:function_end
