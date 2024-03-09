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

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  %3 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  %4 = CallInst [njsf] (:undefined) %1: object, %""(): functionCode, %0: environment, %2: undefined, %3: number, %3: number
// CHECK-NEXT:       ReturnInst %2: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(exports: number): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [?O.prototype: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [?O.prototype]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %O(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %2: object, %4: object, "prototype": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %Foo(): functionCode
// CHECK-NEXT:  %7 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StorePropertyStrictInst %7: object, %6: object, "prototype": string
// CHECK-NEXT:  %9 = HBCAllocObjectFromBufferInst (:object) 3: number, "o0": string, 0: number, "o1": string, 0: number, "o2": string, 0: number
// CHECK-NEXT:        StoreParentInst %7: object, %9: object
// CHECK-NEXT:  %11 = CallInst (:undefined) %6: object, %Foo(): functionCode, %1: environment, %6: object, %9: object
// CHECK-NEXT:  %12 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:        ReturnInst %12: undefined
// CHECK-NEXT:function_end

// CHECK:constructor O(): undefined [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCLoadConstInst (:number) 7: number
// CHECK-NEXT:  %1 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:       PrStoreInst %0: number, %1: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:  %3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       ReturnInst %3: undefined
// CHECK-NEXT:function_end

// CHECK:constructor Foo(): undefined [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %1: environment, [?O.prototype@""]: object
// CHECK-NEXT:  %3 = HBCAllocObjectFromBufferInst (:object) 1: number, "i": string, 0: number
// CHECK-NEXT:       StoreParentInst %2: object, %3: object
// CHECK-NEXT:  %5 = HBCLoadConstInst (:number) 7: number
// CHECK-NEXT:       PrStoreInst %5: number, %3: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:       PrStoreInst %3: object, %0: object, 0: number, "o0": string, false: boolean
// CHECK-NEXT:  %8 = HBCAllocObjectFromBufferInst (:object) 1: number, "i": string, 0: number
// CHECK-NEXT:       StoreParentInst %2: object, %8: object
// CHECK-NEXT:        PrStoreInst %5: number, %8: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:        PrStoreInst %8: object, %0: object, 1: number, "o1": string, false: boolean
// CHECK-NEXT:  %12 = HBCAllocObjectFromBufferInst (:object) 1: number, "i": string, 0: number
// CHECK-NEXT:        StoreParentInst %2: object, %12: object
// CHECK-NEXT:        PrStoreInst %5: number, %12: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:        PrStoreInst %12: object, %0: object, 2: number, "o2": string, false: boolean
// CHECK-NEXT:  %16 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:        ReturnInst %16: undefined
// CHECK-NEXT:function_end
