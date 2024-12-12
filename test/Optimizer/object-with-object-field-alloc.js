/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-lir -O %s | %FileCheckOrRegen --match-full-lines %s

class O {
    i: number;
    constructor() {
        this.i = 7;
    }
}

class Foo {
    o0: O;
    o1: O;
    o2: O;
    constructor() {
        this.o0 = new O();
        this.o1 = new O();
        this.o2 = new O();
    }
}

new Foo();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [?O.prototype: object]

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS1.?O.prototype]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %O(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %2: object, %4: object, "prototype": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %Foo(): functionCode
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:       StorePropertyStrictInst %7: object, %6: object, "prototype": string
// CHECK-NEXT:  %9 = HBCAllocObjectFromBufferInst (:object) "o0": string, 0: number, "o1": string, 0: number, "o2": string, 0: number
// CHECK-NEXT:        TypedStoreParentInst %7: object, %9: object
// CHECK-NEXT:  %11 = CallInst (:undefined) %6: object, %Foo(): functionCode, true: boolean, empty: any, %6: object, %9: object
// CHECK-NEXT:  %12 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:        ReturnInst %12: undefined
// CHECK-NEXT:function_end

// CHECK:constructor O(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCLoadConstInst (:number) 7: number
// CHECK-NEXT:  %1 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:       PrStoreInst %0: number, %1: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:  %3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       ReturnInst %3: undefined
// CHECK-NEXT:function_end

// CHECK:constructor Foo(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %1: environment, [%VS1.?O.prototype]: object
// CHECK-NEXT:  %3 = HBCAllocObjectFromBufferInst (:object) "i": string, 0: number
// CHECK-NEXT:       TypedStoreParentInst %2: object, %3: object
// CHECK-NEXT:  %5 = HBCLoadConstInst (:number) 7: number
// CHECK-NEXT:       PrStoreInst %5: number, %3: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:       PrStoreInst %3: object, %0: object, 0: number, "o0": string, false: boolean
// CHECK-NEXT:  %8 = HBCAllocObjectFromBufferInst (:object) "i": string, 0: number
// CHECK-NEXT:       TypedStoreParentInst %2: object, %8: object
// CHECK-NEXT:        PrStoreInst %5: number, %8: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:        PrStoreInst %8: object, %0: object, 1: number, "o1": string, false: boolean
// CHECK-NEXT:  %12 = HBCAllocObjectFromBufferInst (:object) "i": string, 0: number
// CHECK-NEXT:        TypedStoreParentInst %2: object, %12: object
// CHECK-NEXT:        PrStoreInst %5: number, %12: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:        PrStoreInst %12: object, %0: object, 2: number, "o2": string, false: boolean
// CHECK-NEXT:  %16 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:        ReturnInst %16: undefined
// CHECK-NEXT:function_end
