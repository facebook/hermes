/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function foo(a, b, c) {

}

function bar(x, y, z) {
  foo(x, y, z)
}

// Test that we are passing the 'obj' as the 'this' for the method.
function test_member_access(obj, param) {
  obj.foo(param)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo, bar, test_member_access]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %bar#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %test_member_access#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "test_member_access" : string
// CHECK-NEXT:  %7 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %8 = StoreStackInst undefined : undefined, %7
// CHECK-NEXT:  %9 = LoadStackInst %7
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(a, b, c)#2
// CHECK-NEXT:frame = [a#2, b#2, c#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %a, [a#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %b, [b#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst %c, [c#2], %0
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar#0#1(x, y, z)#3
// CHECK-NEXT:frame = [x#3, y#3, z#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{bar#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %x, [x#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst %y, [y#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst %z, [z#3], %0
// CHECK-NEXT:  %4 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %5 = LoadFrameInst [x#3], %0
// CHECK-NEXT:  %6 = LoadFrameInst [y#3], %0
// CHECK-NEXT:  %7 = LoadFrameInst [z#3], %0
// CHECK-NEXT:  %8 = CallInst %4, undefined : undefined, %5, %6, %7
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_member_access#0#1(obj, param)#4
// CHECK-NEXT:frame = [obj#4, param#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{test_member_access#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %obj, [obj#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst %param, [param#4], %0
// CHECK-NEXT:  %3 = LoadFrameInst [obj#4], %0
// CHECK-NEXT:  %4 = LoadPropertyInst %3, "foo" : string
// CHECK-NEXT:  %5 = LoadFrameInst [param#4], %0
// CHECK-NEXT:  %6 = CallInst %4, %3, %5
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
