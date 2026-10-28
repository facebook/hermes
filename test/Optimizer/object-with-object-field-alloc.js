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
// CHECK-NEXT:  %0 = LIRLoadConstInst (:null) null: null
// CHECK-NEXT:  %1 = LIRAllocTypedNonEnumObjectFromBufferInst (:object) %0: null
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) empty: any, empty: any, %Array(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %1: object, %2: object, "prototype": string
// CHECK-NEXT:  %4 = LIRAllocTypedNonEnumObjectFromBufferInst (:object) %0: null
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) empty: any, empty: any, %"Array 1#"(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %4: object, %5: object, "prototype": string
// CHECK-NEXT:  %7 = LIRAllocTypedNonEnumObjectFromBufferInst (:object) %0: null
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) empty: any, empty: any, %"Array 2#"(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %7: object, %8: object, "prototype": string
// CHECK-NEXT:  %10 = LIRAllocTypedNonEnumObjectFromBufferInst (:object) %0: null
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) empty: any, empty: any, %Symbol(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %10: object, %11: object, "prototype": string
// CHECK-NEXT:  %13 = LIRAllocTypedNonEnumObjectFromBufferInst (:object) %0: null
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) empty: any, empty: any, %Object(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %13: object, %14: object, "prototype": string
// CHECK-NEXT:  %16 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %17 = LIRAllocTypedNonEnumObjectFromBufferInst (:object) %0: null
// CHECK-NEXT:        StoreFrameInst %16: environment, %17: object, [%VS0.?O.prototype]: object
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) empty: any, empty: any, %O(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %17: object, %19: object, "prototype": string
// CHECK-NEXT:  %21 = LIRAllocTypedNonEnumObjectFromBufferInst (:object) %0: null
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %16: environment, %VS0: any, %Foo(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %21: object, %22: object, "prototype": string
// CHECK-NEXT:  %24 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:        ReturnInst %24: undefined
// CHECK-NEXT:function_end

// CHECK:function Array(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       ReturnInst %0: undefined
// CHECK-NEXT:function_end

// CHECK:function "Array 1#"(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       ReturnInst %0: undefined
// CHECK-NEXT:function_end

// CHECK:function "Array 2#"(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       ReturnInst %0: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor Symbol(): any [typed,noReturn]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LIRGetGlobalObjectInst (:object)
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) %0: object, "TypeError": string
// CHECK-NEXT:  %2 = CreateThisInst (:undefined|object) %1: any, %1: any, empty: any
// CHECK-NEXT:  %3 = LIRLoadConstInst (:string) "Symbol is not a constructor": string
// CHECK-NEXT:  %4 = CallInst (:any) %1: any, empty: any, false: boolean, empty: any, %1: any, %2: undefined|object, %3: string
// CHECK-NEXT:  %5 = GetConstructedObjectInst (:object) %2: undefined|object, %4: any
// CHECK-NEXT:       ThrowInst %5: object
// CHECK-NEXT:function_end

// CHECK:function Object(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       ReturnInst %0: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor O(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LIRLoadConstInst (:number) 7: number
// CHECK-NEXT:  %1 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:       PrStoreInst %0: number, %1: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:  %3 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       ReturnInst %3: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor Foo(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %1: environment, [%VS0.?O.prototype]: object
// CHECK-NEXT:  %3 = LIRAllocTypedObjectFromBufferInst (:object) %2: object, "i": string, 0: number
// CHECK-NEXT:  %4 = LIRLoadConstInst (:number) 7: number
// CHECK-NEXT:       PrStoreInst %4: number, %3: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:       PrStoreInst %3: object, %0: object, 0: number, "o0": string, false: boolean
// CHECK-NEXT:  %7 = LIRAllocTypedObjectFromBufferInst (:object) %2: object, "i": string, 0: number
// CHECK-NEXT:       PrStoreInst %4: number, %7: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:       PrStoreInst %7: object, %0: object, 1: number, "o1": string, false: boolean
// CHECK-NEXT:  %10 = LIRAllocTypedObjectFromBufferInst (:object) %2: object, "i": string, 0: number
// CHECK-NEXT:        PrStoreInst %4: number, %10: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:        PrStoreInst %10: object, %0: object, 2: number, "o2": string, false: boolean
// CHECK-NEXT:  %13 = LIRLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:        ReturnInst %13: undefined
// CHECK-NEXT:function_end
