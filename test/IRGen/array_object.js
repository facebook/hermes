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
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(param: any): any
// CHECK-NEXT:frame = [param: any, obj: any, foo: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %param: any
// CHECK-NEXT:       StoreFrameInst %0: any, [param]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [obj]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [foo]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %4: object, "1": string, true: boolean
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [param]: any
// CHECK-NEXT:       StoreNewOwnPropertyInst %6: any, %4: object, "key": string, true: boolean
// CHECK-NEXT:       StoreFrameInst %4: object, [obj]: any
// CHECK-NEXT:  %9 = AllocArrayInst (:object) 4: number, 1: number, 2: number, 3: number, 4: number
// CHECK-NEXT:        StoreFrameInst %9: object, [foo]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [foo]: any
// CHECK-NEXT:        StorePropertyLooseInst %12: any, %11: any, "field": string
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [foo]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:        StorePropertyLooseInst %15: any, %14: any, 5: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
