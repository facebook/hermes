/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir -fno-inline -O0 %s | %FileCheckOrRegen --match-full-lines %s

(function() {

'use strict'

function foo(x, y) {}
$SHBuiltin.call(foo, 11, 12, 13);

})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %1 = StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %""(): any
// CHECK-NEXT:  %3 = CallInst (:any) %2: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %4 = StoreStackInst %3: any, %0: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %0: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = [foo: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [foo]: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %2 = StoreFrameInst %1: closure, [foo]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [foo]: any
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, 11: number, 12: number, 13: number
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(x: any, y: any): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [y]: any
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
