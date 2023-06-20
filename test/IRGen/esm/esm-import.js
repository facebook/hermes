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
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %1 = StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %3 = ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function cjs_module(exports: any, require: any, module: any): any
// CHECK-NEXT:frame = [exports: any, require: any, module: any, Foo: any, defaultFoo: any, x: any, z: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %require: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [require]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %module: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [module]: any
// CHECK-NEXT:  %6 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:  %7 = StoreFrameInst %6: any, [Foo]: any
// CHECK-NEXT:  %8 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "?default": string
// CHECK-NEXT:  %10 = StoreFrameInst %9: any, [defaultFoo]: any
// CHECK-NEXT:  %11 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "foo.js": string
// CHECK-NEXT:  %12 = LoadPropertyInst (:any) %11: any, "x": string
// CHECK-NEXT:  %13 = StoreFrameInst %12: any, [x]: any
// CHECK-NEXT:  %14 = LoadPropertyInst (:any) %11: any, "y": string
// CHECK-NEXT:  %15 = StoreFrameInst %14: any, [z]: any
// CHECK-NEXT:  %16 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "bar.js": string
// CHECK-NEXT:  %17 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
