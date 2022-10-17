/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -commonjs -dump-ir -dump-source-location=loc < %s | %FileCheckOrRegen --match-full-lines %s

x = 10;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = []
// CHECK-NEXT:source location: [<global>:1:1 ... <global>:1:1)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:  %3 = LoadStackInst %1
// CHECK-NEXT:; <global>:1:1
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:function_end

// CHECK:function cjs_module#0#1(exports, require, module)#2
// CHECK-NEXT:frame = [exports#2, require#2, module#2]
// CHECK-NEXT:source location: [<stdin>:10:1 ... <stdin>:10:8)
// CHECK-NEXT:%BB0:
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %0 = CreateScopeInst %S{cjs_module#0#1()#2}
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %1 = StoreFrameInst %exports, [exports#2], %0
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %2 = StoreFrameInst %require, [require#2], %0
// CHECK-NEXT:; <stdin>:10:1
// CHECK-NEXT:  %3 = StoreFrameInst %module, [module#2], %0
// CHECK-NEXT:; <stdin>:10:3
// CHECK-NEXT:  %4 = StorePropertyInst 10 : number, globalObject : object, "x" : string
// CHECK-NEXT:; <stdin>:10:7
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
