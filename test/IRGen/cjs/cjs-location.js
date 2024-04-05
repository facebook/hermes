/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -commonjs -dump-ir -dump-source-location=loc < %s | %FileCheckOrRegen --match-full-lines %s

x = 10;

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:source location: [<global>:1:1 ... <global>:1:1)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:  %3 = LoadStackInst (:any) %1: any
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any, require: any, module: any]

// CHECK:function cjs_module(exports: any, require: any, module: any): any
// CHECK-NEXT:source location: [<stdin>:10:1 ... <stdin>:10:8)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %1 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [%VS1.exports]: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %3 = LoadParamInst (:any) %require: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StoreFrameInst %0: environment, %3: any, [%VS1.require]: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %5 = LoadParamInst (:any) %module: any
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: any, [%VS1.module]: any
// CHECK-NEXT:; <stdin>:10:3
// CHECK-NEXT:       StorePropertyLooseInst 10: number, globalObject: object, "x": string
// CHECK-NEXT:; <stdin>:10:7
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
