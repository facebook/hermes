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
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ID]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i1]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [n]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i2]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [s]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ID#1]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [ID#2]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %ID(): functionCode
// CHECK-NEXT:        StoreFrameInst %9: object, [ID#1]: any
// CHECK-NEXT:  %11 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %11: object, [?ID.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %11: object, %9: object, "prototype": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %"ID 1#"(): functionCode
// CHECK-NEXT:        StoreFrameInst %14: object, [ID#2]: any
// CHECK-NEXT:  %16 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %16: object, [?ID.prototype#1]: object
// CHECK-NEXT:        StorePropertyStrictInst %16: object, %14: object, "prototype": string
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [ID#1]: any
// CHECK-NEXT:  %20 = CheckedTypeCastInst (:object) %19: any, type(object)
// CHECK-NEXT:  %21 = LoadFrameInst (:object) [?ID.prototype]: object
// CHECK-NEXT:  %22 = UnionNarrowTrustedInst (:object) %21: object
// CHECK-NEXT:  %23 = AllocObjectLiteralInst (:object) "val": string, 0: number
// CHECK-NEXT:        StoreParentInst %22: object, %23: object
// CHECK-NEXT:  %25 = CallInst (:any) %20: object, %ID(): functionCode, empty: any, %20: object, %23: object, 1: number
// CHECK-NEXT:        StoreFrameInst %23: object, [i1]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [i1]: any
// CHECK-NEXT:  %28 = CheckedTypeCastInst (:object) %27: any, type(object)
// CHECK-NEXT:  %29 = PrLoadInst (:number) %28: object, 0: number, "val": string
// CHECK-NEXT:        StoreFrameInst %29: number, [n]: any
// CHECK-NEXT:  %31 = LoadFrameInst (:any) [ID#2]: any
// CHECK-NEXT:  %32 = CheckedTypeCastInst (:object) %31: any, type(object)
// CHECK-NEXT:  %33 = LoadFrameInst (:object) [?ID.prototype#1]: object
// CHECK-NEXT:  %34 = UnionNarrowTrustedInst (:object) %33: object
// CHECK-NEXT:  %35 = AllocObjectLiteralInst (:object) "val": string, "": string
// CHECK-NEXT:        StoreParentInst %34: object, %35: object
// CHECK-NEXT:  %37 = CallInst (:any) %32: object, %"ID 1#"(): functionCode, empty: any, %32: object, %35: object, "abc": string
// CHECK-NEXT:        StoreFrameInst %35: object, [i2]: any
// CHECK-NEXT:  %39 = LoadFrameInst (:any) [i2]: any
// CHECK-NEXT:  %40 = CheckedTypeCastInst (:object) %39: any, type(object)
// CHECK-NEXT:  %41 = PrLoadInst (:string) %40: object, 0: number, "val": string
// CHECK-NEXT:        StoreFrameInst %41: string, [s]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ID(val: number): any [typed]
// CHECK-NEXT:frame = [val: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:number) %val: number
// CHECK-NEXT:       StoreFrameInst %1: number, [val]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [val]: any
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:number) %3: any, type(number)
// CHECK-NEXT:       PrStoreInst %4: number, %0: object, 0: number, "val": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "ID 1#"(val: string): any [typed]
// CHECK-NEXT:frame = [val: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:string) %val: string
// CHECK-NEXT:       StoreFrameInst %1: string, [val]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [val]: any
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:string) %3: any, type(string)
// CHECK-NEXT:       PrStoreInst %4: string, %0: object, 0: number, "val": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
