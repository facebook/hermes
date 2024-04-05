/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s

class A {
    y = 7; // Field init for y.
    z: number;
    constructor() {
        // y is initialized before first statement of ctor without super.
        this.z = this.y + 1;
    }
}

// Fields are initialized in order.
class B {
    x = 76;
    y = this.x + 1;
    constructor() {}
}

// Implicit constructor is called, and does field inits.
class C0 {
    x = 775;
    m(): number {
        return this.x + 1;
    }
}

// In a class with a superclass, a method invocation via super
// works and gets the right value; the super() constructor is
// executed before the field inits of the subclass.
class C1 extends C0 {
    y = super.m() + 1;
    constructor() {
        super();
    }
}

// Field initializers may reference variables in their lexical scope.
function f(i: number): number {
    class A {
        x = i * 1000; // Gets the value of i from the invocation.
        constructor() {}
    }
    var a = new A();
    return a.x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any, A: any, B: any, C0: any, C1: any, f: any, <fieldInitFuncVar:A>: object, ?A.prototype: object, <fieldInitFuncVar:B>: object, ?B.prototype: object, <fieldInitFuncVar:C0>: object, ?C0.prototype: object, <fieldInitFuncVar:C1>: object, ?C1.prototype: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.A]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.B]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C0]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C1]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %f(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [%VS1.f]: any
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %1: environment, %<instance_members_initializer:A>(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: object, [%VS1.<fieldInitFuncVar:A>]: object
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %1: environment, %A(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: object, [%VS1.A]: any
// CHECK-NEXT:  %14 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: object, [%VS1.?A.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %14: object, %12: object, "prototype": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %1: environment, %<instance_members_initializer:B>(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: object, [%VS1.<fieldInitFuncVar:B>]: object
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %1: environment, %B(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %19: object, [%VS1.B]: any
// CHECK-NEXT:  %21 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: object, [%VS1.?B.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %21: object, %19: object, "prototype": string
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %1: environment, %<instance_members_initializer:C0>(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: object, [%VS1.<fieldInitFuncVar:C0>]: object
// CHECK-NEXT:  %26 = CreateFunctionInst (:object) %1: environment, %C0(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %26: object, [%VS1.C0]: any
// CHECK-NEXT:  %28 = CreateFunctionInst (:object) %1: environment, %m(): functionCode
// CHECK-NEXT:  %29 = AllocObjectLiteralInst (:object) "m": string, %28: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %29: object, [%VS1.?C0.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %29: object, %26: object, "prototype": string
// CHECK-NEXT:  %32 = LoadFrameInst (:any) %1: environment, [%VS1.C0]: any
// CHECK-NEXT:  %33 = CheckedTypeCastInst (:object) %32: any, type(object)
// CHECK-NEXT:  %34 = CreateFunctionInst (:object) %1: environment, %<instance_members_initializer:C1>(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %34: object, [%VS1.<fieldInitFuncVar:C1>]: object
// CHECK-NEXT:  %36 = CreateFunctionInst (:object) %1: environment, %C1(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %36: object, [%VS1.C1]: any
// CHECK-NEXT:  %38 = LoadFrameInst (:object) %1: environment, [%VS1.?C0.prototype]: object
// CHECK-NEXT:  %39 = PrLoadInst (:object) %38: object, 0: number, "m": string
// CHECK-NEXT:  %40 = AllocObjectLiteralInst (:object) "m": string, %39: object
// CHECK-NEXT:        StoreParentInst %38: object, %40: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %40: object, [%VS1.?C1.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %40: object, %36: object, "prototype": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [i: any, A: any, a: any, <fieldInitFuncVar:A>: object, ?A.prototype: object]

// CHECK:function f(i: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %i: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [%VS2.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.A]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.a]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %"<instance_members_initializer:A> 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS2.<fieldInitFuncVar:A>]: object
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %"A 1#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [%VS2.A]: any
// CHECK-NEXT:  %10 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: object, [%VS2.?A.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %10: object, %8: object, "prototype": string
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [%VS2.A]: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:object) %13: any, type(object)
// CHECK-NEXT:  %15 = LoadFrameInst (:object) %1: environment, [%VS2.?A.prototype]: object
// CHECK-NEXT:  %16 = UnionNarrowTrustedInst (:object) %15: object
// CHECK-NEXT:  %17 = AllocObjectLiteralInst (:object) "x": string, undefined: undefined
// CHECK-NEXT:        StoreParentInst %16: object, %17: object
// CHECK-NEXT:  %19 = CallInst (:any) %14: object, %"A 1#"(): functionCode, empty: any, %14: object, %17: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: object, [%VS2.a]: any
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [%VS2.a]: any
// CHECK-NEXT:  %22 = CheckedTypeCastInst (:object) %21: any, type(object)
// CHECK-NEXT:  %23 = PrLoadInst (:any) %22: object, 0: number, "x": string
// CHECK-NEXT:  %24 = CheckedTypeCastInst (:number) %23: any, type(number)
// CHECK-NEXT:        ReturnInst %24: number
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function <instance_members_initializer:A>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS3: any, %1: environment
// CHECK-NEXT:       PrStoreInst 7: number, %0: any, 0: number, "y": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:constructor A(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS4: any, %1: environment
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %VS1: any, %VS4: any, %2: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:object) %3: environment, [%VS1.<fieldInitFuncVar:A>]: object
// CHECK-NEXT:  %5 = CallInst (:undefined) %4: object, %<instance_members_initializer:A>(): functionCode, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:  %6 = PrLoadInst (:any) %0: object, 0: number, "y": string
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 1: number
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:number) %7: any, type(number)
// CHECK-NEXT:       PrStoreInst %8: number, %0: object, 1: number, "z": string, true: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:function <instance_members_initializer:B>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS5: any, %1: environment
// CHECK-NEXT:       PrStoreInst 76: number, %0: any, 0: number, "x": string, false: boolean
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %0: any, "x": string
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 1: number
// CHECK-NEXT:       PrStoreInst %5: any, %0: any, 1: number, "y": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS6 []

// CHECK:constructor B(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS6: any, %1: environment
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %VS1: any, %VS6: any, %2: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:object) %3: environment, [%VS1.<fieldInitFuncVar:B>]: object
// CHECK-NEXT:  %5 = CallInst (:undefined) %4: object, %<instance_members_initializer:B>(): functionCode, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS7 []

// CHECK:function <instance_members_initializer:C0>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS7: any, %1: environment
// CHECK-NEXT:       PrStoreInst 775: number, %0: any, 0: number, "x": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS8 []

// CHECK:function C0(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS8: any, %1: environment
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %VS1: any, %VS8: any, %2: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:object) %3: environment, [%VS1.<fieldInitFuncVar:C0>]: object
// CHECK-NEXT:  %5 = CallInst (:undefined) %4: object, %<instance_members_initializer:C0>(): functionCode, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS9 []

// CHECK:function m(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS9: any, %1: environment
// CHECK-NEXT:  %3 = PrLoadInst (:any) %0: object, 0: number, "x": string
// CHECK-NEXT:  %4 = BinaryAddInst (:any) %3: any, 1: number
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:scope %VS10 []

// CHECK:function <instance_members_initializer:C1>(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS10: any, %1: environment
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %VS1: any, %VS10: any, %2: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:object) %3: environment, [%VS1.?C0.prototype]: object
// CHECK-NEXT:  %5 = PrLoadInst (:object) %4: object, 0: number, "m": string
// CHECK-NEXT:  %6 = CallInst [njsf] (:any) %5: object, empty: any, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:number) %6: any, type(number)
// CHECK-NEXT:  %8 = BinaryAddInst (:any) %7: number, 1: number
// CHECK-NEXT:  %9 = CheckedTypeCastInst (:number) %8: any, type(number)
// CHECK-NEXT:        PrStoreInst %9: number, %0: any, 1: number, "y": string, false: boolean
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS11 []

// CHECK:constructor C1(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS11: any, %1: environment
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %VS1: any, %VS11: any, %2: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %3: environment, [%VS1.C0]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:object) %4: any, type(object)
// CHECK-NEXT:  %6 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %7 = CallInst (:any) %5: object, empty: any, empty: any, %6: object, %0: object
// CHECK-NEXT:  %8 = ResolveScopeInst (:environment) %VS1: any, %VS11: any, %2: environment
// CHECK-NEXT:  %9 = LoadFrameInst (:object) %8: environment, [%VS1.<fieldInitFuncVar:C1>]: object
// CHECK-NEXT:  %10 = CallInst (:undefined) %9: object, %<instance_members_initializer:C1>(): functionCode, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS12 []

// CHECK:function "<instance_members_initializer:A> 1#"(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS12: any, %1: environment
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %VS2: any, %VS12: any, %2: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %3: environment, [%VS2.i]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:  %6 = BinaryMultiplyInst (:any) %5: number, 1000: number
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:number) %6: any, type(number)
// CHECK-NEXT:       PrStoreInst %7: number, %0: any, 0: number, "x": string, false: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS13 []

// CHECK:constructor "A 1#"(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS2: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS13: any, %1: environment
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %VS2: any, %VS13: any, %2: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:object) %3: environment, [%VS2.<fieldInitFuncVar:A>]: object
// CHECK-NEXT:  %5 = CallInst (:undefined) %4: object, %"<instance_members_initializer:A> 1#"(): functionCode, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
