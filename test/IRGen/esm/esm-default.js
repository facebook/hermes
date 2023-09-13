/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -commonjs -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

export default function() {
  return 400;
}

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
// CHECK-NEXT:frame = [exports: any, require: any, module: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %require: any
// CHECK-NEXT:       StoreFrameInst %2: any, [require]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %module: any
// CHECK-NEXT:       StoreFrameInst %4: any, [module]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:       StorePropertyLooseInst %6: object, %0: any, "?default": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 400: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end
