/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines -check-prefix OPT %s

function foo(x) {
  return [1, 2, ...x, 3, 4];
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [x: any]

// CHECK:function foo(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.x]: any
// CHECK-NEXT:  %4 = AllocStackInst (:number) $nextIndex: any
// CHECK-NEXT:       StoreStackInst 0: number, %4: number
// CHECK-NEXT:  %6 = LoadStackInst (:number) %4: number
// CHECK-NEXT:  %7 = FAddInst (:number) %6: number, 1: number
// CHECK-NEXT:       StoreStackInst %7: number, %4: number
// CHECK-NEXT:  %9 = LoadStackInst (:number) %4: number
// CHECK-NEXT:  %10 = FAddInst (:number) %9: number, 1: number
// CHECK-NEXT:        StoreStackInst %10: number, %4: number
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %13 = AllocArrayInst (:object) 4: number, 1: number, 2: number
// CHECK-NEXT:  %14 = LoadStackInst (:number) %4: number
// CHECK-NEXT:  %15 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %13: object, %12: any, %14: number
// CHECK-NEXT:        StoreStackInst %15: number, %4: number
// CHECK-NEXT:  %17 = LoadStackInst (:number) %4: number
// CHECK-NEXT:        DefineOwnPropertyInst 3: number, %13: object, %17: number, true: boolean
// CHECK-NEXT:  %19 = LoadStackInst (:number) %4: number
// CHECK-NEXT:  %20 = FAddInst (:number) %19: number, 1: number
// CHECK-NEXT:        StoreStackInst %20: number, %4: number
// CHECK-NEXT:  %22 = LoadStackInst (:number) %4: number
// CHECK-NEXT:        DefineOwnPropertyInst 4: number, %13: object, %22: number, true: boolean
// CHECK-NEXT:  %24 = LoadStackInst (:number) %4: number
// CHECK-NEXT:  %25 = FAddInst (:number) %24: number, 1: number
// CHECK-NEXT:        StoreStackInst %25: number, %4: number
// CHECK-NEXT:        ReturnInst %13: object
// CHECK-NEXT:function_end

// OPT:scope %VS0 []

// OPT:function global(): undefined
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// OPT-NEXT:       DeclareGlobalVarInst "foo": string
// OPT-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// OPT-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// OPT-NEXT:       ReturnInst undefined: undefined
// OPT-NEXT:function_end

// OPT:function foo(x: any): object
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = LoadParamInst (:any) %x: any
// OPT-NEXT:  %1 = AllocArrayInst (:object) 4: number, 1: number, 2: number
// OPT-NEXT:  %2 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %1: object, %0: any, 2: number
// OPT-NEXT:       DefineOwnPropertyInst 3: number, %1: object, %2: number, true: boolean
// OPT-NEXT:  %4 = FAddInst (:number) %2: number, 1: number
// OPT-NEXT:       DefineOwnPropertyInst 4: number, %1: object, %4: number, true: boolean
// OPT-NEXT:       ReturnInst %1: object
// OPT-NEXT:function_end
