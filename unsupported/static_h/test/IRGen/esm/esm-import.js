/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -commonjs -dump-ir %s | %FileCheck --match-full-lines %s

// CHECK-LABEL: function cjs_module(exports, require, module)
// CHECK-NEXT: frame = [Foo, defaultFoo, x, z, exports, require, module]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [Foo]
// CHECK-NEXT:   %1 = StoreFrameInst undefined : undefined, [defaultFoo]
// CHECK-NEXT:   %2 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:   %3 = StoreFrameInst undefined : undefined, [z]
// CHECK-NEXT:   %4 = StoreFrameInst %exports, [exports]
// CHECK-NEXT:   %5 = StoreFrameInst %require, [require]
// CHECK-NEXT:   %6 = StoreFrameInst %module, [module]

import * as Foo from 'foo.js';
// CHECK-NEXT:   %7 = CallInst %require, undefined : undefined, "foo.js" : string
// CHECK-NEXT:   %8 = StoreFrameInst %7, [Foo]

import defaultFoo from 'foo.js';
// CHECK-NEXT:   %9 = CallInst %require, undefined : undefined, "foo.js" : string
// CHECK-NEXT:   %10 = LoadPropertyInst %9, "?default" : string
// CHECK-NEXT:   %11 = StoreFrameInst %10, [defaultFoo]

import { x , y as z } from 'foo.js';
// CHECK-NEXT:   %12 = CallInst %require, undefined : undefined, "foo.js" : string
// CHECK-NEXT:   %13 = LoadPropertyInst %12, "x" : string
// CHECK-NEXT:   %14 = StoreFrameInst %13, [x]
// CHECK-NEXT:   %15 = LoadPropertyInst %12, "y" : string
// CHECK-NEXT:   %16 = StoreFrameInst %15, [z]

import 'bar.js';
// CHECK-NEXT:   %17 = CallInst %require, undefined : undefined, "bar.js" : string
// CHECK-NEXT:   %18 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end
