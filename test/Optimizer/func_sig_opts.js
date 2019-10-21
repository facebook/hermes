/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O -fno-inline | %FileCheck %s

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
