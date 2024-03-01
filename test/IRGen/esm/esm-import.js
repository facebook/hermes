/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -commonjs -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

import * as Foo from 'foo.js';

import defaultFoo from 'foo.js';

import { x , y as z } from 'foo.js';

import 'bar.js';

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
// CHECK-NEXT:frame = [exports: any, require: any, module: any, Foo: any, defaultFoo: any, x: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %cjs_module(): any, empty: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: any, [exports]: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %require: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %3: any, [require]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %module: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %5: any, [module]: any
// CHECK-NEXT:  %7 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:       StoreFrameInst %0: environment, %7: any, [Foo]: any
// CHECK-NEXT:  %9 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "?default": string
// CHECK-NEXT:        StoreFrameInst %0: environment, %10: any, [defaultFoo]: any
// CHECK-NEXT:  %12 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:  %13 = LoadPropertyInst (:any) %12: any, "x": string
// CHECK-NEXT:        StoreFrameInst %0: environment, %13: any, [x]: any
// CHECK-NEXT:  %15 = LoadPropertyInst (:any) %12: any, "y": string
// CHECK-NEXT:        StoreFrameInst %0: environment, %15: any, [z]: any
// CHECK-NEXT:  %17 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "bar.js": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
