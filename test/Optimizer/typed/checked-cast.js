/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Check optimization of chained casts.

class Cls {
    next: ?Cls;
}

function next (ptr: ?Cls): Cls {
    return ((((ptr: any): Cls).next : any): Cls);
}

exports.next = next

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %3 = CallInst [njsf] (:undefined) %1: object, %""(): functionCode, %0: environment, undefined: undefined, 0: number, %2: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(exports: object): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:object) %exports: object
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %next(): functionCode
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %Cls(): functionCode
// CHECK-NEXT:  %5 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StorePropertyStrictInst %5: object, %4: object, "prototype": string
// CHECK-NEXT:       StorePropertyStrictInst %3: object, %2: object, "next": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function next(ptr: undefined|null|object): object [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:undefined|null|object) %ptr: undefined|null|object
// CHECK-NEXT:  %1 = CheckedTypeCastInst (:object) %0: undefined|null|object, type(object)
// CHECK-NEXT:  %2 = PrLoadInst (:undefined|null|object) %1: object, 0: number, "next": string
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:object) %2: undefined|null|object, type(object)
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:function Cls(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
