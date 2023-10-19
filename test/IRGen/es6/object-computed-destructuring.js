/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

var {['a']: b} = x;

var {['a']: b, ...rest} = x;

var {[foo()]: b, c: d, ...rest} = x;

var {} = x;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "b": string
// CHECK-NEXT:       DeclareGlobalVarInst "rest": string
// CHECK-NEXT:       DeclareGlobalVarInst "d": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "a": string
// CHECK-NEXT:       StorePropertyLooseInst %6: any, globalObject: object, "b": string
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "a": string
// CHECK-NEXT:        StorePropertyLooseInst %9: any, globalObject: object, "b": string
// CHECK-NEXT:  %11 = AllocObjectInst (:object) 1: number, null: null
// CHECK-NEXT:        StoreNewOwnPropertyInst 0: number, %11: object, "a": string, true: boolean
// CHECK-NEXT:  %13 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %14 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %13: object, %8: any, %11: object
// CHECK-NEXT:        StorePropertyLooseInst %14: any, globalObject: object, "rest": string
// CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %18 = CallInst (:any) %17: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %19 = LoadPropertyInst (:any) %16: any, %18: any
// CHECK-NEXT:        StorePropertyLooseInst %19: any, globalObject: object, "b": string
// CHECK-NEXT:  %21 = LoadPropertyInst (:any) %16: any, "c": string
// CHECK-NEXT:        StorePropertyLooseInst %21: any, globalObject: object, "d": string
// CHECK-NEXT:  %23 = AllocObjectInst (:object) 2: number, null: null
// CHECK-NEXT:        StoreNewOwnPropertyInst 0: number, %23: object, "c": string, true: boolean
// CHECK-NEXT:        StorePropertyLooseInst 0: number, %23: object, %18: any
// CHECK-NEXT:  %26 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %27 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %26: object, %16: any, %23: object
// CHECK-NEXT:        StorePropertyLooseInst %27: any, globalObject: object, "rest": string
// CHECK-NEXT:  %29 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %30 = BinaryEqualInst (:any) %29: any, null: null
// CHECK-NEXT:        CondBranchInst %30: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ThrowTypeErrorInst "Cannot destructure 'undefined' or 'null'.": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %33 = LoadStackInst (:any) %3: any
// CHECK-NEXT:        ReturnInst %33: any
// CHECK-NEXT:function_end
