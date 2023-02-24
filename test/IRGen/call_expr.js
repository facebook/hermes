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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "bar": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "test_member_access": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %bar(): any
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5: closure, globalObject: object, "bar": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %test_member_access(): any
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: closure, globalObject: object, "test_member_access": string
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %10 = StoreStackInst undefined: undefined, %9: any
// CHECK-NEXT:  %11 = LoadStackInst (:any) %9: any
// CHECK-NEXT:  %12 = ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any, c: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %c: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [c]: any
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bar(x: any, y: any, z: any): any
// CHECK-NEXT:frame = [x: any, y: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [z]: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %10 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, %7: any, %8: any, %9: any
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_member_access(obj: any, param: any): any
// CHECK-NEXT:frame = [obj: any, param: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [obj]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %param: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [param]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "foo": string
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [param]: any
// CHECK-NEXT:  %7 = CallInst (:any) %5: any, empty: any, empty: any, %4: any, %6: any
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
