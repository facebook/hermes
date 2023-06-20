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
// CHECK-NEXT:  %1 = StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function cjs_module(exports: any, require: any, module: any): any
// CHECK-NEXT:frame = [exports: any, require: any, module: any, myFun: any, x: any, y: any, z: any, longVariableName: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %require: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [require]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %module: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [module]: any
// CHECK-NEXT:  %6 = StoreFrameInst undefined: undefined, [myFun]: any
// CHECK-NEXT:  %7 = StoreFrameInst undefined: undefined, [x]: any
// CHECK-NEXT:  %8 = StoreFrameInst undefined: undefined, [y]: any
// CHECK-NEXT:  %9 = StoreFrameInst undefined: undefined, [z]: any
// CHECK-NEXT:  %10 = StoreFrameInst undefined: undefined, [longVariableName]: any
// CHECK-NEXT:  %11 = StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %myFun(): any
// CHECK-NEXT:  %13 = StoreFrameInst %12: object, [myFun]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [myFun]: any
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14: any, %0: any, "myFun": string
// CHECK-NEXT:  %16 = StoreFrameInst 1: number, [x]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17: any, %0: any, "x": string
// CHECK-NEXT:  %19 = StoreFrameInst 2: number, [y]: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %21 = StorePropertyLooseInst %20: any, %0: any, "y": string
// CHECK-NEXT:  %22 = StoreFrameInst 4: number, [z]: any
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [z]: any
// CHECK-NEXT:  %24 = StorePropertyLooseInst %23: any, %0: any, "z": string
// CHECK-NEXT:  %25 = StoreFrameInst 3: number, [longVariableName]: any
// CHECK-NEXT:  %26 = StoreFrameInst 4: number, [a]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:  %28 = StorePropertyLooseInst %27: any, %0: any, "a": string
// CHECK-NEXT:  %29 = LoadFrameInst (:any) [longVariableName]: any
// CHECK-NEXT:  %30 = StorePropertyLooseInst %29: any, %0: any, "b": string
// CHECK-NEXT:  %31 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:  %32 = CallBuiltinInst (:any) [HermesBuiltin.exportAll]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, %31: any
// CHECK-NEXT:  %33 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function myFun(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1038: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
