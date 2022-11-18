/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir %s -O -fno-inline | %FileCheckOrRegen %s

"use strict";

function fuzz() {

  function foo(k) {
    return k
  }

  return foo(12)
}

function ctor_test() {

  function foo(k) {
    return k
  }

  return new foo(12)
}

function load_store_test() {

  var k = function(k) { return k }

  function ping() {
      return k(123)
  }

  return ping()
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : string
// CHECK-NEXT:frame = [], globals = [fuzz, ctor_test, load_store_test]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %fuzz() : number
// CHECK-NEXT:  %1 = StorePropertyStrictInst %0 : closure, globalObject : object, "fuzz" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %ctor_test() : object
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2 : closure, globalObject : object, "ctor_test" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %load_store_test() : number
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4 : closure, globalObject : object, "load_store_test" : string
// CHECK-NEXT:  %6 = ReturnInst "use strict" : string
// CHECK-NEXT:function_end

// CHECK:function fuzz() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo() : number
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 12 : number
// CHECK-NEXT:  %2 = ReturnInst 12 : number
// CHECK-NEXT:function_end

// CHECK:function foo(k : number) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 12 : number
// CHECK-NEXT:function_end

// CHECK:function ctor_test() : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %"foo 1#"() : number
// CHECK-NEXT:  %1 = ConstructInst %0 : closure, undefined : undefined, 12 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"(k : number) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 12 : number
// CHECK-NEXT:function_end

// CHECK:function load_store_test() : number
// CHECK-NEXT:frame = [k : closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %ping() : number
// CHECK-NEXT:  %1 = CreateFunctionInst %k() : number
// CHECK-NEXT:  %2 = StoreFrameInst %1 : closure, [k] : closure
// CHECK-NEXT:  %3 = CallInst %0 : closure, undefined : undefined
// CHECK-NEXT:  %4 = ReturnInst 123 : number
// CHECK-NEXT:function_end

// CHECK:function ping() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [k@load_store_test] : closure
// CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 123 : number
// CHECK-NEXT:  %2 = ReturnInst 123 : number
// CHECK-NEXT:function_end

// CHECK:function k(k) : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 123 : number
// CHECK-NEXT:function_end
