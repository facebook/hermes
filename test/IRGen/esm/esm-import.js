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
// CHECK-NEXT:frame = [exports, require, module, Foo, defaultFoo, x, z]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %exports
// CHECK-NEXT:  %1 = StoreFrameInst %0, [exports]
// CHECK-NEXT:  %2 = LoadParamInst %require
// CHECK-NEXT:  %3 = StoreFrameInst %2, [require]
// CHECK-NEXT:  %4 = LoadParamInst %module
// CHECK-NEXT:  %5 = StoreFrameInst %4, [module]
// CHECK-NEXT:  %6 = CallInst %2, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %7 = StoreFrameInst %6, [Foo]
// CHECK-NEXT:  %8 = CallInst %2, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %9 = LoadPropertyInst %8, "?default" : string
// CHECK-NEXT:  %10 = StoreFrameInst %9, [defaultFoo]
// CHECK-NEXT:  %11 = CallInst %2, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %12 = LoadPropertyInst %11, "x" : string
// CHECK-NEXT:  %13 = StoreFrameInst %12, [x]
// CHECK-NEXT:  %14 = LoadPropertyInst %11, "y" : string
// CHECK-NEXT:  %15 = StoreFrameInst %14, [z]
// CHECK-NEXT:  %16 = CallInst %2, undefined : undefined, "bar.js" : string
// CHECK-NEXT:  %17 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
