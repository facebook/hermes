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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_member_access": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %bar(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "bar": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %test_member_access(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "test_member_access": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any, c: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [b]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [c]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function bar(x: any, y: any, z: any): any
// CHECK-NEXT:frame = [x: any, y: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %bar(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [y]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [z]: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [y]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [z]: any
// CHECK-NEXT:  %12 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: any, %10: any, %11: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_member_access(obj: any, param: any): any
// CHECK-NEXT:frame = [obj: any, param: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test_member_access(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [obj]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %param: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [param]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "foo": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [param]: any
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, %6: any, %8: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
