/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

(function() {

return function f(x: any, n: number) {
  // These need CheckedTypeCast on the result of binary ops.
  n += x;
  n -= x;
}

});

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:       StoreStackInst %2: object, %0: any
// CHECK-NEXT:  %4 = LoadStackInst (:any) %0: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:function ""(): any
// CHECK-NEXT:frame = [f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %f(): any
// CHECK-NEXT:       StoreFrameInst %0: object, [f]: any
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function f(x: any, n: number): any [typed]
// CHECK-NEXT:frame = [x: any, n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %2: number, [n]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %4: any, %5: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:number) %6: any
// CHECK-NEXT:       StoreFrameInst %7: number, [n]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [n]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %11 = BinarySubtractInst (:any) %9: any, %10: any
// CHECK-NEXT:  %12 = CheckedTypeCastInst (:number) %11: any
// CHECK-NEXT:        StoreFrameInst %12: number, [n]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
