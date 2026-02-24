/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -dump-ir -Xno-dump-functions=global %s | %FileCheckOrRegen %s --match-full-lines

class ID<T> {
  val: T;

  constructor(val: T) {
    this.val = val;
  }
}

const i1: ID<number> = new ID<number>(1);
const n: number = i1.val;

const i2: ID<string> = new ID<string>('abc');
const s: string = i2.val;

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [exports: any, ID: any, i1: any, n: any, i2: any, s: any, ID#1: any, ID#2: any, ?ID.prototype: object, ?ID.prototype#1: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.ID]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.i1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.n]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.i2]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.s]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.ID#1]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [%VS1.ID#2]: any
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %1: environment, %VS1: any, %ID(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [%VS1.ID#1]: any
// CHECK-NEXT:  %13 = AllocTypedObjectInst (:object) empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [%VS1.?ID.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %13: object, %11: object, "prototype": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %1: environment, %VS1: any, %"ID 1#"(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: object, [%VS1.ID#2]: any
// CHECK-NEXT:  %18 = AllocTypedObjectInst (:object) empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: object, [%VS1.?ID.prototype#1]: object
// CHECK-NEXT:        StorePropertyStrictInst %18: object, %16: object, "prototype": string
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [%VS1.ID#1]: any
// CHECK-NEXT:  %22 = CheckedTypeCastInst (:object) %21: any, type(object)
// CHECK-NEXT:  %23 = LoadFrameInst (:object) %1: environment, [%VS1.?ID.prototype]: object
// CHECK-NEXT:  %24 = UnionNarrowTrustedInst (:object) %23: object
// CHECK-NEXT:  %25 = AllocTypedObjectInst (:object) %24: object, "val": string, 0: number
// CHECK-NEXT:  %26 = CallInst (:any) %22: object, %ID(): functionCode, true: boolean, empty: any, %22: object, %25: object, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %25: object, [%VS1.i1]: any
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [%VS1.i1]: any
// CHECK-NEXT:  %29 = CheckedTypeCastInst (:object) %28: any, type(object)
// CHECK-NEXT:  %30 = PrLoadInst (:number) %29: object, 0: number, "val": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %30: number, [%VS1.n]: any
// CHECK-NEXT:  %32 = LoadFrameInst (:any) %1: environment, [%VS1.ID#2]: any
// CHECK-NEXT:  %33 = CheckedTypeCastInst (:object) %32: any, type(object)
// CHECK-NEXT:  %34 = LoadFrameInst (:object) %1: environment, [%VS1.?ID.prototype#1]: object
// CHECK-NEXT:  %35 = UnionNarrowTrustedInst (:object) %34: object
// CHECK-NEXT:  %36 = AllocTypedObjectInst (:object) %35: object, "val": string, "": string
// CHECK-NEXT:  %37 = CallInst (:any) %33: object, %"ID 1#"(): functionCode, true: boolean, empty: any, %33: object, %36: object, "abc": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %36: object, [%VS1.i2]: any
// CHECK-NEXT:  %39 = LoadFrameInst (:any) %1: environment, [%VS1.i2]: any
// CHECK-NEXT:  %40 = CheckedTypeCastInst (:object) %39: any, type(object)
// CHECK-NEXT:  %41 = PrLoadInst (:string) %40: object, 0: number, "val": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %41: string, [%VS1.s]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [val: any]

// CHECK:base constructor ID(val: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS2: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:number) %val: number
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: number, [%VS2.val]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %2: environment, [%VS2.val]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:number) %5: any, type(number)
// CHECK-NEXT:       PrStoreInst %6: number, %0: object, 0: number, "val": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [val: any]

// CHECK:base constructor "ID 1#"(val: string): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS3: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:string) %val: string
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: string, [%VS3.val]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %2: environment, [%VS3.val]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:string) %5: any, type(string)
// CHECK-NEXT:       PrStoreInst %6: string, %0: object, 0: number, "val": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
