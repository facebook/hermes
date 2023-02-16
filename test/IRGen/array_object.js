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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst (:any) %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(param: any): any
// CHECK-NEXT:frame = [param: any, obj: any, foo: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %param: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [param]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [obj]: any
// CHECK-NEXT:  %3 = StoreFrameInst undefined: undefined, [foo]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [param]: any
// CHECK-NEXT:  %5 = AllocObjectLiteralInst (:object) "1": string, 2: number, "key": string, %4: any
// CHECK-NEXT:  %6 = StoreFrameInst %5: object, [obj]: any
// CHECK-NEXT:  %7 = AllocArrayInst (:object) 4: number, 1: number, 2: number, 3: number, 4: number
// CHECK-NEXT:  %8 = StoreFrameInst %7: object, [foo]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [foo]: any
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10: any, %9: any, "field": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [foo]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: any, %12: any, 5: number
// CHECK-NEXT:  %15 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
