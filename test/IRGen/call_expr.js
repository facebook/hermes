/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "bar" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_member_access" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %foo()
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %bar()
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "bar" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %test_member_access()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "test_member_access" : string
// CHECK-NEXT:  %9 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %10 = StoreStackInst undefined : undefined, %9
// CHECK-NEXT:  %11 = LoadStackInst %9
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:function_end

// CHECK:function foo(a, b, c)
// CHECK-NEXT:frame = [a, b, c]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadParamInst %b
// CHECK-NEXT:  %3 = StoreFrameInst %2, [b]
// CHECK-NEXT:  %4 = LoadParamInst %c
// CHECK-NEXT:  %5 = StoreFrameInst %4, [c]
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function bar(x, y, z)
// CHECK-NEXT:frame = [x, y, z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %x
// CHECK-NEXT:  %1 = StoreFrameInst %0, [x]
// CHECK-NEXT:  %2 = LoadParamInst %y
// CHECK-NEXT:  %3 = StoreFrameInst %2, [y]
// CHECK-NEXT:  %4 = LoadParamInst %z
// CHECK-NEXT:  %5 = StoreFrameInst %4, [z]
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %7 = LoadFrameInst [x]
// CHECK-NEXT:  %8 = LoadFrameInst [y]
// CHECK-NEXT:  %9 = LoadFrameInst [z]
// CHECK-NEXT:  %10 = CallInst %6, undefined : undefined, %7, %8, %9
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function test_member_access(obj, param)
// CHECK-NEXT:frame = [obj, param]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %obj
// CHECK-NEXT:  %1 = StoreFrameInst %0, [obj]
// CHECK-NEXT:  %2 = LoadParamInst %param
// CHECK-NEXT:  %3 = StoreFrameInst %2, [param]
// CHECK-NEXT:  %4 = LoadFrameInst [obj]
// CHECK-NEXT:  %5 = LoadPropertyInst %4, "foo" : string
// CHECK-NEXT:  %6 = LoadFrameInst [param]
// CHECK-NEXT:  %7 = CallInst %5, %4, %6
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
