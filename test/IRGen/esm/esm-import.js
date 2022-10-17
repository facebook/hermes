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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:function_end

// CHECK:function cjs_module#0#1(exports, require, module)#2
// CHECK-NEXT:frame = [exports#2, require#2, module#2, Foo#2, defaultFoo#2, x#2, z#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{cjs_module#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %exports, [exports#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %require, [require#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst %module, [module#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst undefined : undefined, [Foo#2], %0
// CHECK-NEXT:  %5 = StoreFrameInst undefined : undefined, [defaultFoo#2], %0
// CHECK-NEXT:  %6 = StoreFrameInst undefined : undefined, [x#2], %0
// CHECK-NEXT:  %7 = StoreFrameInst undefined : undefined, [z#2], %0
// CHECK-NEXT:  %8 = CallInst %require, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %9 = StoreFrameInst %8, [Foo#2], %0
// CHECK-NEXT:  %10 = CallInst %require, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %11 = LoadPropertyInst %10, "?default" : string
// CHECK-NEXT:  %12 = StoreFrameInst %11, [defaultFoo#2], %0
// CHECK-NEXT:  %13 = CallInst %require, undefined : undefined, "foo.js" : string
// CHECK-NEXT:  %14 = LoadPropertyInst %13, "x" : string
// CHECK-NEXT:  %15 = StoreFrameInst %14, [x#2], %0
// CHECK-NEXT:  %16 = LoadPropertyInst %13, "y" : string
// CHECK-NEXT:  %17 = StoreFrameInst %16, [z#2], %0
// CHECK-NEXT:  %18 = CallInst %require, undefined : undefined, "bar.js" : string
// CHECK-NEXT:  %19 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
