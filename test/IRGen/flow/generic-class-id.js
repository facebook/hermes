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

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, ID: any, i1: any, n: any, i2: any, s: any, ID#1: any, ID#2: any, ?ID.prototype: object, ?ID.prototype#1: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ID]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [n]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i2]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [s]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [ID#1]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [ID#2]: any
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %1: environment, %ID(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [ID#1]: any
// CHECK-NEXT:  %13 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [?ID.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %13: object, %11: object, "prototype": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %1: environment, %"ID 1#"(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: object, [ID#2]: any
// CHECK-NEXT:  %18 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: object, [?ID.prototype#1]: object
// CHECK-NEXT:        StorePropertyStrictInst %18: object, %16: object, "prototype": string
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [ID#1]: any
// CHECK-NEXT:  %22 = CheckedTypeCastInst (:object) %21: any, type(object)
// CHECK-NEXT:  %23 = LoadFrameInst (:object) %1: environment, [?ID.prototype]: object
// CHECK-NEXT:  %24 = UnionNarrowTrustedInst (:object) %23: object
// CHECK-NEXT:  %25 = AllocObjectLiteralInst (:object) "val": string, 0: number
// CHECK-NEXT:        StoreParentInst %24: object, %25: object
// CHECK-NEXT:  %27 = CallInst (:any) %22: object, %ID(): functionCode, empty: any, %22: object, %25: object, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %25: object, [i1]: any
// CHECK-NEXT:  %29 = LoadFrameInst (:any) %1: environment, [i1]: any
// CHECK-NEXT:  %30 = CheckedTypeCastInst (:object) %29: any, type(object)
// CHECK-NEXT:  %31 = PrLoadInst (:number) %30: object, 0: number, "val": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: number, [n]: any
// CHECK-NEXT:  %33 = LoadFrameInst (:any) %1: environment, [ID#2]: any
// CHECK-NEXT:  %34 = CheckedTypeCastInst (:object) %33: any, type(object)
// CHECK-NEXT:  %35 = LoadFrameInst (:object) %1: environment, [?ID.prototype#1]: object
// CHECK-NEXT:  %36 = UnionNarrowTrustedInst (:object) %35: object
// CHECK-NEXT:  %37 = AllocObjectLiteralInst (:object) "val": string, "": string
// CHECK-NEXT:        StoreParentInst %36: object, %37: object
// CHECK-NEXT:  %39 = CallInst (:any) %34: object, %"ID 1#"(): functionCode, empty: any, %34: object, %37: object, "abc": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %37: object, [i2]: any
// CHECK-NEXT:  %41 = LoadFrameInst (:any) %1: environment, [i2]: any
// CHECK-NEXT:  %42 = CheckedTypeCastInst (:object) %41: any, type(object)
// CHECK-NEXT:  %43 = PrLoadInst (:string) %42: object, 0: number, "val": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %43: string, [s]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ID(val: number): any [typed]
// CHECK-NEXT:frame = [val: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %ID(): any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:number) %val: number
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: number, [val]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %2: environment, [val]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:number) %5: any, type(number)
// CHECK-NEXT:       PrStoreInst %6: number, %0: object, 0: number, "val": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "ID 1#"(val: string): any [typed]
// CHECK-NEXT:frame = [val: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %"ID 1#"(): any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:string) %val: string
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: string, [val]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %2: environment, [val]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:string) %5: any, type(string)
// CHECK-NEXT:       PrStoreInst %6: string, %0: object, 0: number, "val": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
