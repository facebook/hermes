/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %" 1#"(): functionCode
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function " 1#"(): any
// CHECK-NEXT:frame = [f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %" 1#"(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %f(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [f]: any
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function f(x: any, n: number): any [typed]
// CHECK-NEXT:frame = [x: any, n: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %" 1#"(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: number, [n]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:number) %6: any, type(number)
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %7: number, %8: any
// CHECK-NEXT:  %10 = CheckedTypeCastInst (:number) %9: any, type(number)
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: number, [n]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [n]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:number) %12: any, type(number)
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %15 = BinarySubtractInst (:any) %13: number, %14: any
// CHECK-NEXT:  %16 = CheckedTypeCastInst (:number) %15: any, type(number)
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: number, [n]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
