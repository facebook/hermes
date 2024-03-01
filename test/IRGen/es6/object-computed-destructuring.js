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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "b": string
// CHECK-NEXT:       DeclareGlobalVarInst "rest": string
// CHECK-NEXT:       DeclareGlobalVarInst "d": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "a": string
// CHECK-NEXT:       StorePropertyLooseInst %7: any, globalObject: object, "b": string
// CHECK-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "a": string
// CHECK-NEXT:        StorePropertyLooseInst %10: any, globalObject: object, "b": string
// CHECK-NEXT:  %12 = AllocObjectInst (:object) 1: number, null: null
// CHECK-NEXT:        StoreNewOwnPropertyInst 0: number, %12: object, "a": string, true: boolean
// CHECK-NEXT:  %14 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %15 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %14: object, %9: any, %12: object
// CHECK-NEXT:        StorePropertyLooseInst %15: any, globalObject: object, "rest": string
// CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %18 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %19 = CallInst (:any) %18: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %17: any, %19: any
// CHECK-NEXT:        StorePropertyLooseInst %20: any, globalObject: object, "b": string
// CHECK-NEXT:  %22 = LoadPropertyInst (:any) %17: any, "c": string
// CHECK-NEXT:        StorePropertyLooseInst %22: any, globalObject: object, "d": string
// CHECK-NEXT:  %24 = AllocObjectInst (:object) 2: number, null: null
// CHECK-NEXT:        StoreNewOwnPropertyInst 0: number, %24: object, "c": string, true: boolean
// CHECK-NEXT:        StorePropertyLooseInst 0: number, %24: object, %19: any
// CHECK-NEXT:  %27 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %28 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %27: object, %17: any, %24: object
// CHECK-NEXT:        StorePropertyLooseInst %28: any, globalObject: object, "rest": string
// CHECK-NEXT:  %30 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %31 = BinaryEqualInst (:any) %30: any, null: null
// CHECK-NEXT:        CondBranchInst %31: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ThrowTypeErrorInst "Cannot destructure 'undefined' or 'null'.": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %34 = LoadStackInst (:any) %4: any
// CHECK-NEXT:        ReturnInst %34: any
// CHECK-NEXT:function_end
