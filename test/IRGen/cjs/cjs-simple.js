/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -commonjs -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

print('done');

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

// CHECK:scope %VS1 [exports: any, require: any, module: any]

// CHECK:function cjs_module(exports: any, require: any, module: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [%VS1.exports]: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %require: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %3: any, [%VS1.require]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %module: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: any, [%VS1.module]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "done": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
