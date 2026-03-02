/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -Werror -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

class C {
  #x: number;
  y: number;
  constructor(x, y) {
    this.#x = x;
    this.y = y;
  }

  method(): number {
    return this.#privateMethod();
  }

  #privateMethod(): number {
    return this.#x;
  }
}

new C(1, 2);

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

// CHECK:scope %VS1 [exports: any, C: any, #privateMethod: object, <fieldInitFuncVar:C>: object, ?C.prototype: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %VS1: any, %#privateMethod(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS1.#privateMethod]: object
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %VS1: any, %<instance_members_initializer:C>(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [%VS1.<fieldInitFuncVar:C>]: object
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %1: environment, %VS1: any, %C(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.C]: any
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %1: environment, %VS1: any, %method(): functionCode
// CHECK-NEXT:  %12 = AllocTypedNonEnumObjectInst (:object) null: null, "method": string, %11: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: object, [%VS1.?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %12: object, %9: object, "prototype": string
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [%VS1.C]: any
// CHECK-NEXT:  %16 = CheckedTypeCastInst (:object) %15: any, type(object)
// CHECK-NEXT:  %17 = LoadFrameInst (:object) %1: environment, [%VS1.?C.prototype]: object
// CHECK-NEXT:  %18 = UnionNarrowTrustedInst (:object) %17: object
// CHECK-NEXT:  %19 = AllocTypedObjectInst (:object) %18: object, private "#x": any, 0: number, "y": string, 0: number
// CHECK-NEXT:  %20 = CallInst (:any) %16: object, %C(): functionCode, true: boolean, empty: any, %16: object, %19: object, 1: number, 2: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function #privateMethod(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS2: any, %1: environment
// CHECK-NEXT:  %3 = PrLoadInst (:number) %0: object, 0: number, private "#x": any
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function <instance_members_initializer:C>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [x: any, y: any]

// CHECK:base constructor C(x: any, y: any): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS4: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS4.x]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS4.y]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:object) %1: environment, [%VS1.<fieldInitFuncVar:C>]: object
// CHECK-NEXT:  %8 = CallInst (:undefined) %7: object, %<instance_members_initializer:C>(): functionCode, true: boolean, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS4.x]: any
// CHECK-NEXT:  %10 = CheckedTypeCastInst (:number) %9: any, type(number)
// CHECK-NEXT:        PrStoreInst %10: number, %0: object, 0: number, private "#x": any, true: boolean
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS4.y]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:number) %12: any, type(number)
// CHECK-NEXT:        PrStoreInst %13: number, %0: object, 1: number, "y": string, true: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:function method(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS5: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %1: environment, [%VS1.#privateMethod]: object
// CHECK-NEXT:  %4 = CallInst [njsf] (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end
