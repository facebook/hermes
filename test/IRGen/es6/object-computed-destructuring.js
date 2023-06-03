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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "b": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "rest": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "d": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "a": string
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6: any, globalObject: object, "b": string
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "a": string
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9: any, globalObject: object, "b": string
// CHECK-NEXT:  %11 = AllocObjectLiteralInst (:object) "a": string, 0: number
// CHECK-NEXT:  %12 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %13 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, %12: object, %8: any, %11: object
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: any, globalObject: object, "rest": string
// CHECK-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %17 = CallInst (:any) %16: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) %15: any, %17: any
// CHECK-NEXT:  %19 = StorePropertyLooseInst %18: any, globalObject: object, "b": string
// CHECK-NEXT:  %20 = LoadPropertyInst (:any) %15: any, "c": string
// CHECK-NEXT:  %21 = StorePropertyLooseInst %20: any, globalObject: object, "d": string
// CHECK-NEXT:  %22 = AllocObjectLiteralInst (:object) "c": string, 0: number
// CHECK-NEXT:  %23 = StorePropertyLooseInst 0: number, %22: object, %17: any
// CHECK-NEXT:  %24 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %25 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, %24: object, %15: any, %22: object
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25: any, globalObject: object, "rest": string
// CHECK-NEXT:  %27 = TryLoadGlobalPropertyInst (:any) globalObject: object, "x": string
// CHECK-NEXT:  %28 = BinaryEqualInst (:any) %27: any, null: null
// CHECK-NEXT:  %29 = CondBranchInst %28: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %30 = ThrowTypeErrorInst "Cannot destructure 'undefined' or 'null'.": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %31 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %32 = ReturnInst %31: any
// CHECK-NEXT:function_end
