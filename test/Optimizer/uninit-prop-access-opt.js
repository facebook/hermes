/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -fno-inline -typed -dump-ir -O %s | %FileCheckOrRegen --match-full-lines %s

class O {
    i: number;
    constructor() {
        this.i = 7;
    }
}

class Foo {
    o: O;
    y: number;
    constructor() {
        this.y = this.o.i;
        this.o = new O();
    }
}

function f(): void {
    var f: Foo = new Foo();
    var i: number = f.o.i;
    // The question this test is raising is whether the second occurrence of
    // f.o.i below, dominated by the first above, would have a ThrowIfInst for
    // the f.o access.  As of this writing, it does.  It would be nice to be
    // able to optimize that away.  But it requires reasoning about heap field
    // values.  In this case, simple reasoning, since there are no intervening
    // calls that might change the heap; it could even be CSE's.
    i += f.o.i;
}

print(f());

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %2 = CallInst [njsf] (:undefined) %1: object, %""(): functionCode, %0: environment, undefined: undefined, 0: number, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [O: object, Foo: undefined|object, ?O.prototype: object, ?Foo.prototype: object]

// CHECK:function ""(exports: number): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.Foo]: undefined|object
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %f(): functionCode
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %O(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS1.O]: object
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.?O.prototype]: object
// CHECK-NEXT:       StorePropertyStrictInst %6: object, %4: object, "prototype": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %1: environment, %Foo(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.Foo]: undefined|object
// CHECK-NEXT:  %11 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [%VS1.?Foo.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %11: object, %9: object, "prototype": string
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %15 = CallInst [njsf] (:undefined) %3: object, %f(): functionCode, %1: environment, undefined: undefined, 0: number
// CHECK-NEXT:  %16 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined, undefined: undefined, undefined: undefined
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f(): undefined [allCallsitesKnownInStrictMode,typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:undefined|object) %0: environment, [%VS1.Foo]: undefined|object
// CHECK-NEXT:  %2 = CheckedTypeCastInst (:object) %1: undefined|object, type(object)
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %0: environment, [%VS1.?Foo.prototype]: object
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any, "o": string, 0: number, "y": string, 0: number
// CHECK-NEXT:       StoreParentInst %3: object, %4: object
// CHECK-NEXT:  %6 = CallInst (:undefined) %2: object, %Foo(): functionCode, empty: any, %2: object, %4: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:constructor O(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:       PrStoreInst 7: number, %0: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:constructor Foo(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = PrLoadInst (:object) %0: object, 0: number, "o": string
// CHECK-NEXT:  %3 = PrLoadInst (:number) %2: object, 0: number, "i": string
// CHECK-NEXT:       PrStoreInst %3: number, %0: object, 1: number, "y": string, true: boolean
// CHECK-NEXT:  %5 = LoadFrameInst (:object) %1: environment, [%VS1.O]: object
// CHECK-NEXT:  %6 = LoadFrameInst (:object) %1: environment, [%VS1.?O.prototype]: object
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) empty: any, "i": string, 0: number
// CHECK-NEXT:       StoreParentInst %6: object, %7: object
// CHECK-NEXT:  %9 = CallInst (:undefined) %5: object, %O(): functionCode, empty: any, %5: object, %7: object
// CHECK-NEXT:        PrStoreInst %7: object, %0: object, 0: number, "o": string, false: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
