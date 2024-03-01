/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function foo(param) {
  var obj = {"1" : 2, "key" : param};

  var foo = [1,2,3,4];

  obj.field = foo;

  foo[5] = obj;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function foo(param: any): any
// CHECK-NEXT:frame = [param: any, obj: any, foo: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %param: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [param]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [foo]: any
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %6: object, "1": string, true: boolean
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [param]: any
// CHECK-NEXT:       StoreNewOwnPropertyInst %8: any, %6: object, "key": string, true: boolean
// CHECK-NEXT:        StoreFrameInst %1: environment, %6: object, [obj]: any
// CHECK-NEXT:  %11 = AllocArrayInst (:object) 4: number, 1: number, 2: number, 3: number, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [foo]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [foo]: any
// CHECK-NEXT:        StorePropertyLooseInst %14: any, %13: any, "field": string
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [foo]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:        StorePropertyLooseInst %17: any, %16: any, 5: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
