/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O  | %FileCheckOrRegen %s

// Make sure that we are promoting t to a constant value in bar()
function foo(p1) {
  var t = 123;

  function bar() {
    return t;
  }

  return bar;
}

foo()()

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo() : closure
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function foo(p1) : closure
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %bar() : number
// CHECK-NEXT:  %1 = ReturnInst %0 : closure
// CHECK-NEXT:function_end

// CHECK:function bar() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 123 : number
// CHECK-NEXT:function_end
