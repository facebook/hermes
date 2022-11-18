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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [foo, bar, test_member_access]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %bar()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %test_member_access()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "test_member_access" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadStackInst %6
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function foo(a, b, c)
// CHECK-NEXT:frame = [a, b, c]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:  %1 = StoreFrameInst %b, [b]
// CHECK-NEXT:  %2 = StoreFrameInst %c, [c]
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar(x, y, z)
// CHECK-NEXT:frame = [x, y, z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %x, [x]
// CHECK-NEXT:  %1 = StoreFrameInst %y, [y]
// CHECK-NEXT:  %2 = StoreFrameInst %z, [z]
// CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %4 = LoadFrameInst [x]
// CHECK-NEXT:  %5 = LoadFrameInst [y]
// CHECK-NEXT:  %6 = LoadFrameInst [z]
// CHECK-NEXT:  %7 = CallInst %3, undefined : undefined, %4, %5, %6
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_member_access(obj, param)
// CHECK-NEXT:frame = [obj, param]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %obj, [obj]
// CHECK-NEXT:  %1 = StoreFrameInst %param, [param]
// CHECK-NEXT:  %2 = LoadFrameInst [obj]
// CHECK-NEXT:  %3 = LoadPropertyInst %2, "foo" : string
// CHECK-NEXT:  %4 = LoadFrameInst [param]
// CHECK-NEXT:  %5 = CallInst %3, %2, %4
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
