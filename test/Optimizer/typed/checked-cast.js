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

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %next(): functionCode
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %Cls(): functionCode
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:       StorePropertyStrictInst %4: object, %3: object, "prototype": string
// CHECK-NEXT:       StorePropertyStrictInst %2: object, %1: object, "next": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function next(ptr: undefined|null|object): object [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:undefined|null|object) %ptr: undefined|null|object
// CHECK-NEXT:  %1 = CheckedTypeCastInst (:object) %0: undefined|null|object, type(object)
// CHECK-NEXT:  %2 = PrLoadInst (:undefined|null|object) %1: object, 0: number, "next": string
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:object) %2: undefined|null|object, type(object)
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:function Cls(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
