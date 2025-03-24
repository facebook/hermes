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

// CHECK:scope %VS0 [?O.prototype: object]

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocTypedObjectInst (:object) empty: any
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: object, [%VS0.?O.prototype]: object
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) empty: any, empty: any, %O(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %1: object, %3: object, "prototype": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %Foo(): functionCode
// CHECK-NEXT:  %6 = AllocTypedObjectInst (:object) empty: any
// CHECK-NEXT:       StorePropertyStrictInst %6: object, %5: object, "prototype": string
// CHECK-NEXT:  %8 = HBCAllocObjectFromBufferInst (:object) "o0": string, uninit: uninit, "o1": string, uninit: uninit, "o2": string, uninit: uninit
// CHECK-NEXT:  %9 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %8: object, %6: object
// CHECK-NEXT:  %10 = CallInst (:undefined) %5: object, %Foo(): functionCode, true: boolean, empty: any, %5: object, %8: object
// CHECK-NEXT:  %11 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:        ReturnInst %11: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor O(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = HBCLoadConstInst (:number) 7: number
// CHECK-NEXT:  %1 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:       PrStoreInst %0: number, %1: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:  %3 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       ReturnInst %3: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor Foo(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %1: environment, [%VS0.?O.prototype]: object
// CHECK-NEXT:  %3 = HBCAllocObjectFromBufferInst (:object) "i": string, 0: number
// CHECK-NEXT:  %4 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %3: object, %2: object
// CHECK-NEXT:  %5 = HBCLoadConstInst (:number) 7: number
// CHECK-NEXT:       PrStoreInst %5: number, %3: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:       PrStoreInst %3: object, %0: object, 0: number, "o0": string, false: boolean
// CHECK-NEXT:  %8 = HBCAllocObjectFromBufferInst (:object) "i": string, 0: number
// CHECK-NEXT:  %9 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %8: object, %2: object
// CHECK-NEXT:        PrStoreInst %5: number, %8: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:        PrStoreInst %8: object, %0: object, 1: number, "o1": string, false: boolean
// CHECK-NEXT:  %12 = HBCAllocObjectFromBufferInst (:object) "i": string, 0: number
// CHECK-NEXT:  %13 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %12: object, %2: object
// CHECK-NEXT:        PrStoreInst %5: number, %12: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:        PrStoreInst %12: object, %0: object, 2: number, "o2": string, false: boolean
// CHECK-NEXT:  %16 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:        ReturnInst %16: undefined
// CHECK-NEXT:function_end
