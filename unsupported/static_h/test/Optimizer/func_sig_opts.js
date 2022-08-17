/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O -fno-inline | %FileCheck %s

"use strict";

//CHECK-LABEL:function foo(x : number, y : number)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BinaryOperatorInst '+', 1 : number, 2 : number
//CHECK-NEXT:  %1 = ReturnInst %0 : number
//CHECK-NEXT:function_end

//CHECK-LABEL:function bar(x, y)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = BinaryOperatorInst '+', %x, %y
//CHECK-NEXT:  %1 = ReturnInst %0 : string|number
//CHECK-NEXT:function_end

function main(p) {
  function foo(x, y) { return x + y; }
  function bar(x, y) { return x + y; }

  // capture bar.
  p.p = bar;

  return foo(1,2) + bar (1,2)
}

//CHECK-LABEL:function return_types(p)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %builder()
//CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:  %2 = CallInst %0 : closure, undefined : undefined
//CHECK-NEXT:  %3 = BinaryOperatorInst '+', %1 : number, %2 : number
//CHECK-NEXT:  %4 = ReturnInst %3 : number
//CHECK-NEXT:function_end
function return_types(p) {
  function builder() { return k * 1 }

  return builder() + builder()
}



function test_unused_and_duplicate_params() {
  //CHECK-LABEL:function foo2(a, b, c, d) : string|number
  //CHECK-NEXT:frame = []
  //CHECK-NEXT:%BB0:
  //CHECK-NEXT:  %0 = BinaryOperatorInst '+', %a, 2 : number
  //CHECK-NEXT:  %1 = BinaryOperatorInst '+', %0 : string|number, %c
  //CHECK-NEXT:  %2 = ReturnInst %1 : string|number
  //CHECK-NEXT:function_end
  function foo2(a, b, c, d) {
    return a + b + c
  }

  //CHECK-LABEL:function bar1(e) : undefined
  //CHECK-NEXT:frame = []
  //CHECK-NEXT:%BB0:
  //CHECK-NEXT:  %0 = LoadFrameInst [foo2@test_unused_and_duplicate_params] : closure
  //CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, %e, 2 : number, 1 : number, undefined : undefined, undefined : undefined, undefined : undefined
  //CHECK-NEXT:  %2 = ReturnInst undefined : undefined
  //CHECK-NEXT:function_end
  function bar1(e) {
    foo2(e ,2, 1, 4, 5, "")
  }

  //CHECK-LABEL:function bar2(e) : undefined
  //CHECK-NEXT:frame = []
  //CHECK-NEXT:%BB0:
  //CHECK-NEXT:  %0 = LoadFrameInst [foo2@test_unused_and_duplicate_params] : closure
  //CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, %e, 2 : number, 3 : number, undefined : undefined, undefined : undefined
  //CHECK-NEXT:  %2 = ReturnInst undefined : undefined
  //CHECK-NEXT:function_end
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
//CHECK-LABEL:function test_rest_arguments()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %baz()
//CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 100 : number
//CHECK-NEXT:  %2 = ReturnInst %1
//CHECK-NEXT:function_end

//CHECK-LABEL:function baz()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.copyRestArgs] : number, undefined : undefined, 0 : number
//CHECK-NEXT:  %1 = ReturnInst %0
//CHECK-NEXT:function_end

function test_generator() {
  function* gen(x) { return x; }
  return gen(1);
}

//CHECK-LABEL:function test_generator() : object
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %gen() : object
//CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 1 : number
//CHECK-NEXT:  %2 = ReturnInst %1 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function gen() : object
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_gen()
//CHECK-NEXT:  %1 = ReturnInst %0 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_gen(x)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StartGeneratorInst
//CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
//CHECK-NEXT:  %2 = ResumeGeneratorInst %1
//CHECK-NEXT:  %3 = LoadStackInst %1
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %5 = ReturnInst %x
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %6 = ReturnInst %2
//CHECK-NEXT:function_end

function test_async() {
  async function asyncFn(x) { return x; }
  return asyncFn(1);
}

//CHECK-LABEL:function test_async()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %asyncFn()
//CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 1 : number
//CHECK-NEXT:  %2 = ReturnInst %1
//CHECK-NEXT:function_end

//CHECK-LABEL:function asyncFn()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateArgumentsInst
//CHECK-NEXT:  %1 = CreateFunctionInst %?anon_0_asyncFn() : object
//CHECK-NEXT:  %2 = GetBuiltinClosureInst [HermesBuiltin.spawnAsync] : number
//CHECK-NEXT:  %3 = CallInst %2 : closure, undefined : undefined, %1 : closure, %this, %0 : object
//CHECK-NEXT:  %4 = ReturnInst %3
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_asyncFn() : object
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateGeneratorInst %?anon_0_?anon_0_asyncFn()
//CHECK-NEXT:  %1 = ReturnInst %0 : object
//CHECK-NEXT:function_end

//CHECK-LABEL:function ?anon_0_?anon_0_asyncFn(x)
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StartGeneratorInst
//CHECK-NEXT:  %1 = AllocStackInst $?anon_0_isReturn_prologue
//CHECK-NEXT:  %2 = ResumeGeneratorInst %1
//CHECK-NEXT:  %3 = LoadStackInst %1
//CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %5 = ReturnInst %x
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %6 = ReturnInst %2
//CHECK-NEXT:function_end
