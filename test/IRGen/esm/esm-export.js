/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -commonjs -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

export default function myFun() {
  return 1038;
}

export var x = 1;

export var y = 2, z = 4;

var longVariableName = 3;
var a = 4;

export { a, longVariableName as b }

export * from 'foo.js';

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:function cjs_module(exports: any, require: any, module: any): any
// CHECK-NEXT:frame = [exports: any, require: any, module: any, myFun: any, x: any, y: any, z: any, longVariableName: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %cjs_module(): any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [exports]: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %require: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %3: any, [require]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %module: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: any, [module]: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [myFun]: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [y]: any
// CHECK-NEXT:        StoreFrameInst %0: environment, undefined: undefined, [z]: any
// CHECK-NEXT:        StoreFrameInst %0: environment, undefined: undefined, [longVariableName]: any
// CHECK-NEXT:        StoreFrameInst %0: environment, undefined: undefined, [a]: any
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %myFun(): functionCode
// CHECK-NEXT:        StoreFrameInst %0: environment, %13: object, [myFun]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %0: environment, [myFun]: any
// CHECK-NEXT:        StorePropertyLooseInst %15: any, %1: any, "myFun": string
// CHECK-NEXT:        StoreFrameInst %0: environment, 1: number, [x]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %0: environment, [x]: any
// CHECK-NEXT:        StorePropertyLooseInst %18: any, %1: any, "x": string
// CHECK-NEXT:        StoreFrameInst %0: environment, 2: number, [y]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %0: environment, [y]: any
// CHECK-NEXT:        StorePropertyLooseInst %21: any, %1: any, "y": string
// CHECK-NEXT:        StoreFrameInst %0: environment, 4: number, [z]: any
// CHECK-NEXT:  %24 = LoadFrameInst (:any) %0: environment, [z]: any
// CHECK-NEXT:        StorePropertyLooseInst %24: any, %1: any, "z": string
// CHECK-NEXT:        StoreFrameInst %0: environment, 3: number, [longVariableName]: any
// CHECK-NEXT:        StoreFrameInst %0: environment, 4: number, [a]: any
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %0: environment, [a]: any
// CHECK-NEXT:        StorePropertyLooseInst %28: any, %1: any, "a": string
// CHECK-NEXT:  %30 = LoadFrameInst (:any) %0: environment, [longVariableName]: any
// CHECK-NEXT:        StorePropertyLooseInst %30: any, %1: any, "b": string
// CHECK-NEXT:  %32 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:  %33 = CallBuiltinInst (:any) [HermesBuiltin.exportAll]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any, %32: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function myFun(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %cjs_module(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %myFun(): any, %0: environment
// CHECK-NEXT:       ReturnInst 1038: number
// CHECK-NEXT:function_end
