/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-std-globals -O0 -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

class Box {
  value: number;

  constructor() {
    this.value = 0;
  }

  @Hermes.final
  identity<T>(x: T): T {
    return x;
  }
}

let box: Box = new Box();

// Test explicit type arguments.
let a: number = box.identity<number>(42);
let b: string = box.identity<string>("hello");

// Test type inference.
let c: number = box.identity(42);
let d: string = box.identity("hello");

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any, Box: any, box: any, a: any, b: any, c: any, d: any, identity: object, identity#1: object, ?Box.prototype: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.Box]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.box]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.b]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.c]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.d]: any
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %1: environment, %VS1: any, %Box(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: object, [%VS1.Box]: any
// CHECK-NEXT:  %12 = AllocTypedNonEnumObjectInst (:object) null: null
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %1: environment, %VS1: any, %identity(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [%VS1.identity]: object
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %1: environment, %VS1: any, %"identity 1#"(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: object, [%VS1.identity#1]: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: object, [%VS1.?Box.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %12: object, %10: object, "prototype": string
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [%VS1.Box]: any
// CHECK-NEXT:  %20 = CheckedTypeCastInst (:object) %19: any, type(object)
// CHECK-NEXT:  %21 = LoadFrameInst (:object) %1: environment, [%VS1.?Box.prototype]: object
// CHECK-NEXT:  %22 = UnionNarrowTrustedInst (:object) %21: object
// CHECK-NEXT:  %23 = AllocTypedObjectInst (:object) %22: object, "value": string, 0: number
// CHECK-NEXT:  %24 = CallInst (:any) %20: object, %Box(): functionCode, true: boolean, empty: any, %20: object, %23: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %23: object, [%VS1.box]: any
// CHECK-NEXT:  %26 = LoadFrameInst (:any) %1: environment, [%VS1.box]: any
// CHECK-NEXT:  %27 = CheckedTypeCastInst (:object) %26: any, type(object)
// CHECK-NEXT:  %28 = LoadFrameInst (:object) %1: environment, [%VS1.identity]: object
// CHECK-NEXT:  %29 = CallInst [njsf] (:any) %28: object, %identity(): functionCode, true: boolean, empty: any, undefined: undefined, %27: object, 42: number
// CHECK-NEXT:  %30 = CheckedTypeCastInst (:number) %29: any, type(number)
// CHECK-NEXT:        StoreFrameInst %1: environment, %30: number, [%VS1.a]: any
// CHECK-NEXT:  %32 = LoadFrameInst (:any) %1: environment, [%VS1.box]: any
// CHECK-NEXT:  %33 = CheckedTypeCastInst (:object) %32: any, type(object)
// CHECK-NEXT:  %34 = LoadFrameInst (:object) %1: environment, [%VS1.identity#1]: object
// CHECK-NEXT:  %35 = CallInst [njsf] (:any) %34: object, %"identity 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, %33: object, "hello": string
// CHECK-NEXT:  %36 = CheckedTypeCastInst (:string) %35: any, type(string)
// CHECK-NEXT:        StoreFrameInst %1: environment, %36: string, [%VS1.b]: any
// CHECK-NEXT:  %38 = LoadFrameInst (:any) %1: environment, [%VS1.box]: any
// CHECK-NEXT:  %39 = CheckedTypeCastInst (:object) %38: any, type(object)
// CHECK-NEXT:  %40 = LoadFrameInst (:object) %1: environment, [%VS1.identity]: object
// CHECK-NEXT:  %41 = CallInst [njsf] (:any) %40: object, %identity(): functionCode, true: boolean, empty: any, undefined: undefined, %39: object, 42: number
// CHECK-NEXT:  %42 = CheckedTypeCastInst (:number) %41: any, type(number)
// CHECK-NEXT:        StoreFrameInst %1: environment, %42: number, [%VS1.c]: any
// CHECK-NEXT:  %44 = LoadFrameInst (:any) %1: environment, [%VS1.box]: any
// CHECK-NEXT:  %45 = CheckedTypeCastInst (:object) %44: any, type(object)
// CHECK-NEXT:  %46 = LoadFrameInst (:object) %1: environment, [%VS1.identity#1]: object
// CHECK-NEXT:  %47 = CallInst [njsf] (:any) %46: object, %"identity 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, %45: object, "hello": string
// CHECK-NEXT:  %48 = CheckedTypeCastInst (:string) %47: any, type(string)
// CHECK-NEXT:        StoreFrameInst %1: environment, %48: string, [%VS1.d]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:base constructor Box(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS2: any, %1: environment
// CHECK-NEXT:       PrStoreInst 0: number, %0: object, 0: number, "value": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [x: any]

// CHECK:function identity(x: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [%VS3.x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS3.x]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [x: any]

// CHECK:function "identity 1#"(x: string): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:string) %x: string
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: string, [%VS4.x]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS4.x]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:string) %4: any, type(string)
// CHECK-NEXT:       ReturnInst %5: string
// CHECK-NEXT:function_end
