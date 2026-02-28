/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -dump-ir -Xno-dump-functions=global %s | %FileCheckOrRegen %s --match-full-lines

class A<T> {
  foo(): void {
    print('foo');
  }
}

var a: A<number> = new A<number>();
a.foo();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [exports: any, a: any, A: any, ?A.prototype: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.A]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %VS1: any, %A(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.A]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %VS1: any, %foo(): functionCode
// CHECK-NEXT:  %9 = AllocTypedNonEnumObjectInst (:object) null: null, "foo": string, %8: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.?A.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %9: object, %6: object, "prototype": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.A]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = LoadFrameInst (:object) %1: environment, [%VS1.?A.prototype]: object
// CHECK-NEXT:  %15 = UnionNarrowTrustedInst (:object) %14: object
// CHECK-NEXT:  %16 = AllocTypedObjectInst (:object) %15: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: object, [%VS1.a]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [%VS1.a]: any
// CHECK-NEXT:  %19 = CheckedTypeCastInst (:object) %18: any, type(object)
// CHECK-NEXT:  %20 = TypedLoadParentInst (:object) %19: object
// CHECK-NEXT:  %21 = PrLoadInst (:object) %20: object, 0: number, "foo": string
// CHECK-NEXT:  %22 = CallInst [njsf] (:any) %21: object, %foo(): functionCode, true: boolean, empty: any, undefined: undefined, %19: object
// CHECK-NEXT:  %23 = CheckedTypeCastInst (:undefined) %22: any, type(undefined)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function A(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function foo(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
