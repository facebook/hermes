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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = AllocStackInst (:number) $nextIndex: any
// CHECK-NEXT:       StoreStackInst 0: number, %2: number
// CHECK-NEXT:  %4 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: number, 1: number
// CHECK-NEXT:       StoreStackInst %5: any, %2: number
// CHECK-NEXT:  %7 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %7: number, 1: number
// CHECK-NEXT:       StoreStackInst %8: any, %2: number
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %11 = AllocArrayInst (:object) 4: number, 1: number, 2: number
// CHECK-NEXT:  %12 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %13 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: object, %10: any, %12: number
// CHECK-NEXT:        StoreStackInst %13: number, %2: number
// CHECK-NEXT:  %15 = LoadStackInst (:number) %2: number
// CHECK-NEXT:        StoreOwnPropertyInst 3: number, %11: object, %15: number, true: boolean
// CHECK-NEXT:  %17 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %18 = BinaryAddInst (:any) %17: number, 1: number
// CHECK-NEXT:        StoreStackInst %18: any, %2: number
// CHECK-NEXT:  %20 = LoadStackInst (:number) %2: number
// CHECK-NEXT:        StoreOwnPropertyInst 4: number, %11: object, %20: number, true: boolean
// CHECK-NEXT:  %22 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %23 = BinaryAddInst (:any) %22: number, 1: number
// CHECK-NEXT:        StoreStackInst %23: any, %2: number
// CHECK-NEXT:        ReturnInst %11: object
// CHECK-NEXT:function_end

// OPT:function global(): undefined
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:       DeclareGlobalVarInst "foo": string
// OPT-NEXT:  %1 = CreateFunctionInst (:object) %foo(): functionCode
// OPT-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// OPT-NEXT:       ReturnInst undefined: undefined
// OPT-NEXT:function_end

// OPT:function foo(x: any): object
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = LoadParamInst (:any) %x: any
// OPT-NEXT:  %1 = AllocArrayInst (:object) 4: number, 1: number, 2: number
// OPT-NEXT:  %2 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: object, %0: any, 2: number
// OPT-NEXT:       StoreOwnPropertyInst 3: number, %1: object, %2: number, true: boolean
// OPT-NEXT:  %4 = FAddInst (:number) %2: number, 1: number
// OPT-NEXT:       StoreOwnPropertyInst 4: number, %1: object, %4: number, true: boolean
// OPT-NEXT:       ReturnInst %1: object
// OPT-NEXT:function_end
