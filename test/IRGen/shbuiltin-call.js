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

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [exports: any, foo: any]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS1.foo]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS1.foo]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:object) %6: any, type(object)
// CHECK-NEXT:  %8 = CallInst (:any) %7: object, empty: any, false: boolean, empty: any, undefined: undefined, 11: number, 12: number, 13: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
