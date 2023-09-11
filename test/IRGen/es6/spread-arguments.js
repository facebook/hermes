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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(fn: any, x: any): any
// CHECK-NEXT:frame = [fn: any, x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %fn: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [fn]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [fn]: any
// CHECK-NEXT:  %5 = AllocStackInst (:number) $nextIndex: any
// CHECK-NEXT:  %6 = StoreStackInst 0: number, %5: number
// CHECK-NEXT:  %7 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %8 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %9 = LoadStackInst (:number) %5: number
// CHECK-NEXT:  %10 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %8: object, %7: any, %9: number
// CHECK-NEXT:  %11 = StoreStackInst %10: number, %5: number
// CHECK-NEXT:  %12 = CallBuiltinInst (:any) [HermesBuiltin.apply]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: any, %8: object, undefined: undefined
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [fn]: any
// CHECK-NEXT:  %14 = AllocStackInst (:number) $nextIndex: any
// CHECK-NEXT:  %15 = StoreStackInst 0: number, %14: number
// CHECK-NEXT:  %16 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %17 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %18 = LoadStackInst (:number) %14: number
// CHECK-NEXT:  %19 = CallBuiltinInst (:number) [HermesBuiltin.arraySpread]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %17: object, %16: any, %18: number
// CHECK-NEXT:  %20 = StoreStackInst %19: number, %14: number
// CHECK-NEXT:  %21 = CallBuiltinInst (:any) [HermesBuiltin.apply]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %13: any, %17: object
// CHECK-NEXT:  %22 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// OPT:function global(): undefined
// OPT-NEXT:frame = []
// OPT-NEXT:%BB0:
// OPT-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// OPT-NEXT:  %1 = CreateFunctionInst (:object) %foo(): undefined
// OPT-NEXT:  %2 = StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// OPT-NEXT:  %3 = ReturnInst undefined: undefined
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
// OPT-NEXT:  %8 = ReturnInst undefined: undefined
// OPT-NEXT:function_end
