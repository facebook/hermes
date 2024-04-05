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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any, require: any, module: any, myFun: any, x: any, y: any, z: any, longVariableName: any, a: any]

// CHECK:function cjs_module(exports: any, require: any, module: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [%VS1.exports]: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %require: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %3: any, [%VS1.require]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %module: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: any, [%VS1.module]: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS1.myFun]: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS1.x]: any
// CHECK-NEXT:       StoreFrameInst %0: environment, undefined: undefined, [%VS1.y]: any
// CHECK-NEXT:        StoreFrameInst %0: environment, undefined: undefined, [%VS1.z]: any
// CHECK-NEXT:        StoreFrameInst %0: environment, undefined: undefined, [%VS1.longVariableName]: any
// CHECK-NEXT:        StoreFrameInst %0: environment, undefined: undefined, [%VS1.a]: any
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %myFun(): functionCode
// CHECK-NEXT:        StoreFrameInst %0: environment, %13: object, [%VS1.myFun]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %0: environment, [%VS1.myFun]: any
// CHECK-NEXT:        StorePropertyLooseInst %15: any, %1: any, "myFun": string
// CHECK-NEXT:        StoreFrameInst %0: environment, 1: number, [%VS1.x]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %0: environment, [%VS1.x]: any
// CHECK-NEXT:        StorePropertyLooseInst %18: any, %1: any, "x": string
// CHECK-NEXT:        StoreFrameInst %0: environment, 2: number, [%VS1.y]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %0: environment, [%VS1.y]: any
// CHECK-NEXT:        StorePropertyLooseInst %21: any, %1: any, "y": string
// CHECK-NEXT:        StoreFrameInst %0: environment, 4: number, [%VS1.z]: any
// CHECK-NEXT:  %24 = LoadFrameInst (:any) %0: environment, [%VS1.z]: any
// CHECK-NEXT:        StorePropertyLooseInst %24: any, %1: any, "z": string
// CHECK-NEXT:        StoreFrameInst %0: environment, 3: number, [%VS1.longVariableName]: any
// CHECK-NEXT:        StoreFrameInst %0: environment, 4: number, [%VS1.a]: any
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %0: environment, [%VS1.a]: any
// CHECK-NEXT:        StorePropertyLooseInst %28: any, %1: any, "a": string
// CHECK-NEXT:  %30 = LoadFrameInst (:any) %0: environment, [%VS1.longVariableName]: any
// CHECK-NEXT:        StorePropertyLooseInst %30: any, %1: any, "b": string
// CHECK-NEXT:  %32 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:  %33 = CallBuiltinInst (:any) [HermesBuiltin.exportAll]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: any, %32: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function myFun(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       ReturnInst 1038: number
// CHECK-NEXT:function_end
