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

// CHECK:function global#0()#1 : string
// CHECK-NEXT:frame = [], globals = [fuzz, ctor_test, load_store_test]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %fuzz#0#1()#2 : number, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "fuzz" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %ctor_test#0#1()#4 : object, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "ctor_test" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %load_store_test#0#1()#6 : number, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "load_store_test" : string
// CHECK-NEXT:  %7 = ReturnInst "use strict" : string
// CHECK-NEXT:function_end

// CHECK:function fuzz#0#1()#2 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{fuzz#0#1()#2}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#1#2()#3 : number, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, 12 : number
// CHECK-NEXT:  %3 = ReturnInst 12 : number
// CHECK-NEXT:function_end

// CHECK:function foo#1#2(k : number)#3 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#1#2()#3}
// CHECK-NEXT:  %1 = ReturnInst 12 : number
// CHECK-NEXT:function_end

// CHECK:function ctor_test#0#1()#4 : object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{ctor_test#0#1()#4}
// CHECK-NEXT:  %1 = CreateFunctionInst %"foo 1#"#1#4()#5 : number, %0
// CHECK-NEXT:  %2 = ConstructInst %1 : closure, undefined : undefined, 12 : number
// CHECK-NEXT:  %3 = ReturnInst %2 : object
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"#1#4(k : number)#5 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"foo 1#"#1#4()#5}
// CHECK-NEXT:  %1 = ReturnInst 12 : number
// CHECK-NEXT:function_end

// CHECK:function load_store_test#0#1()#6 : number
// CHECK-NEXT:frame = [k#6 : closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{load_store_test#0#1()#6}
// CHECK-NEXT:  %1 = CreateFunctionInst %ping#1#6()#7 : number, %0
// CHECK-NEXT:  %2 = CreateFunctionInst %k#1#6()#8 : number, %0
// CHECK-NEXT:  %3 = StoreFrameInst %2 : closure, [k#6] : closure, %0
// CHECK-NEXT:  %4 = CallInst %1 : closure, undefined : undefined
// CHECK-NEXT:  %5 = ReturnInst 123 : number
// CHECK-NEXT:function_end

// CHECK:function ping#1#6()#7 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{ping#1#6()#7}
// CHECK-NEXT:  %1 = LoadFrameInst [k#6@load_store_test] : closure, %0
// CHECK-NEXT:  %2 = CallInst %1 : closure, undefined : undefined, 123 : number
// CHECK-NEXT:  %3 = ReturnInst 123 : number
// CHECK-NEXT:function_end

// CHECK:function k#1#6(k)#8 : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{k#1#6()#8}
// CHECK-NEXT:  %1 = ReturnInst 123 : number
// CHECK-NEXT:function_end
