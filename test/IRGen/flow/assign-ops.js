/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions=f -Werror -O0 -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

(function() {

return function f(x: any, n: number) {
  // These need CheckedTypeCast on the result of binary ops.
  n += x;
  n -= x;
}

});

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [f: any]

// CHECK:scope %VS1 [x: any, n: any]

// CHECK:function f(x: any, n: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: number, [%VS1.n]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS1.n]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:number) %6: any, type(number)
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %7: number, %8: any
// CHECK-NEXT:  %10 = CheckedTypeCastInst (:number) %9: any, type(number)
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: number, [%VS1.n]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.n]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:number) %12: any, type(number)
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %15 = BinarySubtractInst (:any) %13: number, %14: any
// CHECK-NEXT:  %16 = CheckedTypeCastInst (:number) %15: any, type(number)
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: number, [%VS1.n]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
