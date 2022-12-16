/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -commonjs -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

import * as Foo from 'foo.js';

import defaultFoo from 'foo.js';

import { x , y as z } from 'foo.js';

import 'bar.js';

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
// CHECK-NEXT:  %2 = LoadStackInst %0
// CHECK-NEXT:  %3 = ReturnInst %2
// CHECK-NEXT:function_end

// CHECK:function cjs_module(exports, require, module)
// CHECK-NEXT:frame = [Foo, defaultFoo, x, z, exports, require, module]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [Foo]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [defaultFoo]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %3 = StoreFrameInst undefined : undefined, [z]
// CHECK-NEXT:  %4 = LoadParamInst %exports
// CHECK-NEXT:  %5 = StoreFrameInst %4, [exports]
// CHECK-NEXT:  %6 = LoadParamInst %require
// CHECK-NEXT:  %7 = StoreFrameInst %6, [require]
// CHECK-NEXT:  %8 = LoadParamInst %module
// CHECK-NEXT:  %9 = StoreFrameInst %8, [module]
// CHECK-NEXT:  %10 = CallInst %6, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %11 = StoreFrameInst %10, [Foo]
// CHECK-NEXT:  %12 = CallInst %6, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %13 = LoadPropertyInst %12, "?default" : string
// CHECK-NEXT:  %14 = StoreFrameInst %13, [defaultFoo]
// CHECK-NEXT:  %15 = CallInst %6, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %16 = LoadPropertyInst %15, "x" : string
// CHECK-NEXT:  %17 = StoreFrameInst %16, [x]
// CHECK-NEXT:  %18 = LoadPropertyInst %15, "y" : string
// CHECK-NEXT:  %19 = StoreFrameInst %18, [z]
// CHECK-NEXT:  %20 = CallInst %6, undefined : undefined, "bar.js" : string
// CHECK-NEXT:  %21 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
