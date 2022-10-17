/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -commonjs -dump-ir %S/cjs-multiple-1.js %S/cjs-multiple-2.js | %FileCheckOrRegen --match-full-lines %s

print('done 1');

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
// CHECK-NEXT:frame = [exports#2, require#2, module#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{cjs_module#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %exports, [exports#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst %require, [require#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst %module, [module#2], %0
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined, "done 1" : string
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "cjs_module 1#"#0#1(exports, require, module)#3
// CHECK-NEXT:frame = [exports#3, require#3, module#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"cjs_module 1#"#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %exports, [exports#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst %require, [require#3], %0
// CHECK-NEXT:  %3 = StoreFrameInst %module, [module#3], %0
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined, "done 2" : string
// CHECK-NEXT:  %6 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
