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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "bar": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_member_access": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %VS0: any, %bar(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "bar": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %VS0: any, %test_member_access(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "test_member_access": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [a: any, b: any, c: any]

// CHECK:function foo(a: any, b: any, c: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS1.b]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [%VS1.c]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any, y: any, z: any]

// CHECK:function bar(x: any, y: any, z: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS2.y]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %z: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [%VS2.z]: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS2.y]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [%VS2.z]: any
// CHECK-NEXT:  %12 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %9: any, %10: any, %11: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [obj: any, param: any]

// CHECK:function test_member_access(obj: any, param: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.obj]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %param: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS3.param]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS3.obj]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "foo": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS3.param]: any
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, %6: any, %8: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
