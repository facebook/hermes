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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): any
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [x]: any
// CHECK-NEXT:  %2 = AllocStackInst (:number) $nextIndex: any
// CHECK-NEXT:  %3 = StoreStackInst 0: number, %2: number
// CHECK-NEXT:  %4 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: number, 1: number
// CHECK-NEXT:  %6 = StoreStackInst %5: any, %2: number
// CHECK-NEXT:  %7 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %7: number, 1: number
// CHECK-NEXT:  %9 = StoreStackInst %8: any, %2: number
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %11 = AllocArrayInst (:object) 4: number, 1: number, 2: number
// CHECK-NEXT:  %12 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %13 = CallBuiltinInst (:any) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: object, %10: any, %12: number
// CHECK-NEXT:  %14 = StoreStackInst %13: any, %2: number
// CHECK-NEXT:  %15 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %16 = StoreOwnPropertyInst 3: number, %11: object, %15: number, true: boolean
// CHECK-NEXT:  %17 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %18 = BinaryAddInst (:any) %17: number, 1: number
// CHECK-NEXT:  %19 = StoreStackInst %18: any, %2: number
// CHECK-NEXT:  %20 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %21 = StoreOwnPropertyInst 4: number, %11: object, %20: number, true: boolean
// CHECK-NEXT:  %22 = LoadStackInst (:number) %2: number
// CHECK-NEXT:  %23 = BinaryAddInst (:any) %22: number, 1: number
// CHECK-NEXT:  %24 = StoreStackInst %23: any, %2: number
// CHECK-NEXT:  %25 = ReturnInst %11: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %26 = UnreachableInst
// CHECK-NEXT:function_end

// OPT:function global(): undefined
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// OPT-NEXT:  %1 = CreateFunctionInst (:object) %foo(): object
// OPT-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// OPT-NEXT:  %3 = ReturnInst undefined: undefined
// OPT-NEXT:function_end

// OPT:function foo(x: any): object
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = LoadParamInst (:any) %x: any
// OPT-NEXT:  %1 = AllocArrayInst (:object) 4: number, 1: number, 2: number
// OPT-NEXT:  %2 = CallBuiltinInst (:any) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: object, %0: any, 2: number
// OPT-NEXT:  %3 = StoreOwnPropertyInst 3: number, %1: object, %2: any, true: boolean
// OPT-NEXT:  %4 = BinaryAddInst (:string|number) %2: any, 1: number
// OPT-NEXT:  %5 = StoreOwnPropertyInst 4: number, %1: object, %4: string|number, true: boolean
// OPT-NEXT:  %6 = ReturnInst %1: object
// OPT-NEXT:function_end
