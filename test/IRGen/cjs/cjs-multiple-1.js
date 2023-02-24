/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -commonjs -dump-ir %S/cjs-multiple-1.js %S/cjs-multiple-2.js | %FileCheckOrRegen --match-full-lines %s

print('done 1');

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
// CHECK-NEXT:frame = [exports: any, require: any, module: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %require: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [require]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %module: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [module]: any
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, "done 1": string
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "cjs_module 1#"(exports: any, require: any, module: any): any
// CHECK-NEXT:frame = [exports: any, require: any, module: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %require: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [require]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %module: any
// CHECK-NEXT:  %5 = StoreFrameInst %4: any, [module]: any
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, "done 2": string
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
