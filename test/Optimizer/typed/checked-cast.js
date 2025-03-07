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
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %next(): functionCode
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) empty: any, empty: any, %Cls(): functionCode
// CHECK-NEXT:  %3 = AllocTypedObjectInst (:object) empty: any
// CHECK-NEXT:       StorePropertyStrictInst %3: object, %2: object, "prototype": string
// CHECK-NEXT:       StorePropertyStrictInst %1: object, %0: object, "next": string
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
