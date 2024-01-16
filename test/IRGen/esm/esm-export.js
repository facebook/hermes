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
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = LoadStackInst (:any) %0: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function cjs_module(exports: any, require: any, module: any): any
// CHECK-NEXT:frame = [exports: any, require: any, module: any, myFun: any, x: any, y: any, z: any, longVariableName: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %require: any
// CHECK-NEXT:       StoreFrameInst %2: any, [require]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %module: any
// CHECK-NEXT:       StoreFrameInst %4: any, [module]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [myFun]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [y]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [z]: any
// CHECK-NEXT:        StoreFrameInst undefined: undefined, [longVariableName]: any
// CHECK-NEXT:        StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %myFun(): functionCode
// CHECK-NEXT:        StoreFrameInst %12: object, [myFun]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [myFun]: any
// CHECK-NEXT:        StorePropertyLooseInst %14: any, %0: any, "myFun": string
// CHECK-NEXT:        StoreFrameInst 1: number, [x]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:        StorePropertyLooseInst %17: any, %0: any, "x": string
// CHECK-NEXT:        StoreFrameInst 2: number, [y]: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:        StorePropertyLooseInst %20: any, %0: any, "y": string
// CHECK-NEXT:        StoreFrameInst 4: number, [z]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:        StorePropertyLooseInst %23: any, %0: any, "z": string
// CHECK-NEXT:        StoreFrameInst 3: number, [longVariableName]: any
// CHECK-NEXT:        StoreFrameInst 4: number, [a]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:        StorePropertyLooseInst %27: any, %0: any, "a": string
// CHECK-NEXT:  %29 = LoadFrameInst (:any) [longVariableName]: any
// CHECK-NEXT:        StorePropertyLooseInst %29: any, %0: any, "b": string
// CHECK-NEXT:  %31 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:  %32 = CallBuiltinInst (:any) [HermesBuiltin.exportAll]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, %31: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function myFun(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 1038: number
// CHECK-NEXT:function_end
