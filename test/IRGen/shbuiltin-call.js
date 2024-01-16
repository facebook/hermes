/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions="" -typed -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s

function foo(x, y) {}
$SHBuiltin.call(foo, 11, 12, 13);

// Auto-generated content below. Please do not modify manually.

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, foo: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:       StoreFrameInst %2: object, [foo]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [foo]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:object) %4: any, type(object)
// CHECK-NEXT:  %6 = CallInst (:any) %5: object, empty: any, empty: any, undefined: undefined, 11: number, 12: number, 13: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
