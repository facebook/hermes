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
// CHECK-NEXT:  %2 = CallInst [njsf] (:undefined) %1: object, %""(): functionCode, true: boolean, %0: environment, undefined: undefined, 0: number, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [O: object, Foo: undefined|object, <fieldInitFuncVar:O>: object, ?O.prototype: object, <fieldInitFuncVar:Foo>: object, ?Foo.prototype: object]

// CHECK:function ""(exports: number): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.Foo]: undefined|object
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %f(): functionCode
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %<instance_members_initializer:O>(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS1.<fieldInitFuncVar:O>]: object
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %O(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.O]: object
// CHECK-NEXT:  %8 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [%VS1.?O.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %8: object, %6: object, "prototype": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %1: environment, %<instance_members_initializer:Foo>(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [%VS1.<fieldInitFuncVar:Foo>]: object
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %1: environment, %Foo(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [%VS1.Foo]: undefined|object
// CHECK-NEXT:  %15 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: object, [%VS1.?Foo.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %15: object, %13: object, "prototype": string
// CHECK-NEXT:  %18 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %19 = CallInst [njsf] (:undefined) %3: object, %f(): functionCode, true: boolean, %1: environment, undefined: undefined, 0: number
// CHECK-NEXT:  %20 = CallInst (:any) %18: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, undefined: undefined
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f(): undefined [allCallsitesKnownInStrictMode,typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:undefined|object) %0: environment, [%VS1.Foo]: undefined|object
// CHECK-NEXT:  %2 = CheckedTypeCastInst (:object) %1: undefined|object, type(object)
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %0: environment, [%VS1.?Foo.prototype]: object
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any, "o": string, 0: number, "y": string, 0: number
// CHECK-NEXT:       TypedStoreParentInst %3: object, %4: object
// CHECK-NEXT:  %6 = CallInst (:undefined) %2: object, %Foo(): functionCode, true: boolean, %0: environment, %2: object, %4: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function <instance_members_initializer:O>(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:constructor O(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %1: environment, [%VS1.<fieldInitFuncVar:O>]: object
// CHECK-NEXT:  %3 = CallInst (:undefined) %2: object, %<instance_members_initializer:O>(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:       PrStoreInst 7: number, %0: object, 0: number, "i": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function <instance_members_initializer:Foo>(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:constructor Foo(): undefined [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %1: environment, [%VS1.<fieldInitFuncVar:Foo>]: object
// CHECK-NEXT:  %3 = CallInst (:undefined) %2: object, %<instance_members_initializer:Foo>(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = PrLoadInst (:object) %0: object, 0: number, "o": string
// CHECK-NEXT:  %5 = PrLoadInst (:number) %4: object, 0: number, "i": string
// CHECK-NEXT:       PrStoreInst %5: number, %0: object, 1: number, "y": string, true: boolean
// CHECK-NEXT:  %7 = LoadFrameInst (:object) %1: environment, [%VS1.O]: object
// CHECK-NEXT:  %8 = LoadFrameInst (:object) %1: environment, [%VS1.?O.prototype]: object
// CHECK-NEXT:  %9 = AllocObjectLiteralInst (:object) empty: any, "i": string, 0: number
// CHECK-NEXT:        TypedStoreParentInst %8: object, %9: object
// CHECK-NEXT:  %11 = CallInst (:undefined) %7: object, %O(): functionCode, true: boolean, %1: environment, %7: object, %9: object
// CHECK-NEXT:        PrStoreInst %9: object, %0: object, 0: number, "o": string, false: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
