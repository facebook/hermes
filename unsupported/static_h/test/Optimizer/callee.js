/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O -fno-inline | %FileCheck %s

"use strict";

//CHECK-LABEL:function fuzz() : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %foo() : number
//CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 12 : number
//CHECK-NEXT:  %2 = ReturnInst 12 : number
//CHECK-NEXT:function_end

function fuzz() {

  function foo(k) {
    return k
  }

  return foo(12)
}

//CHECK-LABEL:function ctor_test() : object
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %"foo 1#"() : number
//CHECK-NEXT:  %1 = ConstructInst %0 : closure, undefined : undefined, 12 : number
//CHECK-NEXT:  %2 = ReturnInst %1 : object
//CHECK-NEXT:function_end
function ctor_test() {

  function foo(k) {
    return k
  }

  return new foo(12)
}

//CHECK-LABEL:function ping() : number
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = LoadFrameInst [k@load_store_test] : closure
//CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 123 : number
//CHECK-NEXT:  %2 = ReturnInst 123 : number
//CHECK-NEXT:function_end
function load_store_test() {

  var k = function(k) { return k }

  function ping() {
      return k(123)
  }

  return ping()
}

