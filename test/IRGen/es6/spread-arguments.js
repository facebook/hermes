/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -dump-ir -O %s | %FileCheckOrRegen --match-full-lines -check-prefix=OPT %s

function foo(fn, x) {
  fn(...x);
  new fn(...x);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function foo(fn: any, x: any): any
// CHECK-NEXT:frame = [fn: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %fn: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [fn]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [x]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [fn]: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $nextIndex: any
// CHECK-NEXT:       StoreStackInst 0: number, %7: number
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %10 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %11 = LoadStackInst (:number) %7: number
// CHECK-NEXT:  %12 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %10: object, %9: any, %11: number
// CHECK-NEXT:        StoreStackInst %12: number, %7: number
// CHECK-NEXT:  %14 = CallBuiltinInst (:any) [HermesBuiltin.apply]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %6: any, %10: object, undefined: undefined
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [fn]: any
// CHECK-NEXT:  %16 = AllocStackInst (:number) $nextIndex: any
// CHECK-NEXT:        StoreStackInst 0: number, %16: number
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [x]: any
// CHECK-NEXT:  %19 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %20 = LoadStackInst (:number) %16: number
// CHECK-NEXT:  %21 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %19: object, %18: any, %20: number
// CHECK-NEXT:        StoreStackInst %21: number, %16: number
// CHECK-NEXT:  %23 = CallBuiltinInst (:any) [HermesBuiltin.apply]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %15: any, %19: object
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// OPT:function global(): undefined
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// OPT-NEXT:       DeclareGlobalVarInst "foo": string
// OPT-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// OPT-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// OPT-NEXT:       ReturnInst undefined: undefined
// OPT-NEXT:function_end

// OPT:function foo(fn: any, x: any): undefined
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = LoadParamInst (:any) %fn: any
// OPT-NEXT:  %1 = LoadParamInst (:any) %x: any
// OPT-NEXT:  %2 = AllocArrayInst (:object) 0: number
// OPT-NEXT:  %3 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: object, %1: any, 0: number
// OPT-NEXT:  %4 = CallBuiltinInst (:any) [HermesBuiltin.apply]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, %2: object, undefined: undefined
// OPT-NEXT:  %5 = AllocArrayInst (:object) 0: number
// OPT-NEXT:  %6 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: object, %1: any, 0: number
// OPT-NEXT:  %7 = CallBuiltinInst (:any) [HermesBuiltin.apply]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %0: any, %5: object
// OPT-NEXT:       ReturnInst undefined: undefined
// OPT-NEXT:function_end
