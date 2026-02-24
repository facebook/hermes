/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -strict -dump-ir %s     -O  | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermesc -hermes-parser -strict -dump-ir %s     -O0 | %FileCheckOrRegen %s --match-full-lines

// Unoptimized:

// Optimized:

// Optimized:

function foo(p1) {
  var t = p1;
  var k = t;
  var z = k;
  var y = z;
  return y;
}

foo()

function test2(p1, p2) {
  var x = p1 + p2;
  /// No need to load X so many times.
  print(x + 1, x + 2, x + 3, x + 4, x + 5, x + 6)
}

// Auto-generated content below. Please do not modify manually.

// OPT-CHECK:function global(): any
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// OPT-CHECK-NEXT:       DeclareGlobalVarInst "test2": string
// OPT-CHECK-NEXT:  %2 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// OPT-CHECK-NEXT:       StorePropertyStrictInst %2: object, globalObject: object, "foo": string
// OPT-CHECK-NEXT:  %4 = CreateFunctionInst (:object) empty: any, empty: any, %test2(): functionCode
// OPT-CHECK-NEXT:       StorePropertyStrictInst %4: object, globalObject: object, "test2": string
// OPT-CHECK-NEXT:  %6 = LoadPropertyInst (:any) globalObject: object, "foo": string
// OPT-CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// OPT-CHECK-NEXT:       ReturnInst %7: any
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function foo(p1: any): any
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// OPT-CHECK-NEXT:       ReturnInst %0: any
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function test2(p1: any, p2: any): undefined
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// OPT-CHECK-NEXT:  %1 = LoadParamInst (:any) %p2: any
// OPT-CHECK-NEXT:  %2 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// OPT-CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-CHECK-NEXT:  %4 = BinaryAddInst (:string|number) %2: string|number|bigint, 1: number
// OPT-CHECK-NEXT:  %5 = BinaryAddInst (:string|number) %2: string|number|bigint, 2: number
// OPT-CHECK-NEXT:  %6 = BinaryAddInst (:string|number) %2: string|number|bigint, 3: number
// OPT-CHECK-NEXT:  %7 = BinaryAddInst (:string|number) %2: string|number|bigint, 4: number
// OPT-CHECK-NEXT:  %8 = BinaryAddInst (:string|number) %2: string|number|bigint, 5: number
// OPT-CHECK-NEXT:  %9 = BinaryAddInst (:string|number) %2: string|number|bigint, 6: number
// OPT-CHECK-NEXT:  %10 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: string|number, %5: string|number, %6: string|number, %7: string|number, %8: string|number, %9: string|number
// OPT-CHECK-NEXT:        ReturnInst undefined: undefined
// OPT-CHECK-NEXT:function_end

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "test2": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %3: object, globalObject: object, "foo": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %test2(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %5: object, globalObject: object, "test2": string
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %7: any
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %7: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [p1: any, t: any, k: any, z: any, y: any]

// CHECK:function foo(p1: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p1: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.p1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.k]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.z]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.y]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS1.p1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: any, [%VS1.t]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS1.t]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [%VS1.k]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.k]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: any, [%VS1.z]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS1.z]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: any, [%VS1.y]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS1.y]: any
// CHECK-NEXT:        ReturnInst %16: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [p1: any, p2: any, x: any]

// CHECK:function test2(p1: any, p2: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p1: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.p1]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %p2: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS2.p2]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS2.p1]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS2.p2]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %7: any, %8: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: any, [%VS2.x]: any
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %13 = BinaryAddInst (:any) %12: any, 1: number
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %15 = BinaryAddInst (:any) %14: any, 2: number
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %17 = BinaryAddInst (:any) %16: any, 3: number
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %19 = BinaryAddInst (:any) %18: any, 4: number
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %21 = BinaryAddInst (:any) %20: any, 5: number
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %23 = BinaryAddInst (:any) %22: any, 6: number
// CHECK-NEXT:  %24 = CallInst (:any) %11: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %13: any, %15: any, %17: any, %19: any, %21: any, %23: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
