/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function f1(t) {
    var {...a} = t;
    return a;
}

function f2(t) {
    var {a, b, ...rest} = t;
    return rest;
}

function f3(t) {
    var a, rest;
    ({a, ...rest} = t);
}

function f4(o, t) {
    var a;
    ({a, ...o.rest} = t);
}

function f5(o) {
    var a, rest;
    ({a, a,  ...rest} = o);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2": string
// CHECK-NEXT:       DeclareGlobalVarInst "f3": string
// CHECK-NEXT:       DeclareGlobalVarInst "f4": string
// CHECK-NEXT:       DeclareGlobalVarInst "f5": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %f1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "f1": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %f2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "f2": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %f3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "f3": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %f4(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "f4": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %f5(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "f5": string
// CHECK-NEXT:  %16 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %16: any
// CHECK-NEXT:  %18 = LoadStackInst (:any) %16: any
// CHECK-NEXT:        ReturnInst %18: any
// CHECK-NEXT:function_end

// CHECK:function f1(t: any): any
// CHECK-NEXT:frame = [t: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [t]: any
// CHECK-NEXT:  %6 = BinaryEqualInst (:any) %5: any, null: null
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ThrowTypeErrorInst "Cannot destructure 'undefined' or 'null'.": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %10 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: object, %5: any, undefined: undefined
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [a]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [a]: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:function f2(t: any): any
// CHECK-NEXT:frame = [t: any, a: any, b: any, rest: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [b]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [rest]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [t]: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "a": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: any, [a]: any
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %7: any, "b": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [b]: any
// CHECK-NEXT:  %12 = AllocObjectInst (:object) 2: number, null: null
// CHECK-NEXT:        StoreNewOwnPropertyInst 0: number, %12: object, "a": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst 0: number, %12: object, "b": string, true: boolean
// CHECK-NEXT:  %15 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %16 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %15: object, %7: any, %12: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: any, [rest]: any
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [rest]: any
// CHECK-NEXT:        ReturnInst %18: any
// CHECK-NEXT:function_end

// CHECK:function f3(t: any): any
// CHECK-NEXT:frame = [t: any, a: any, rest: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f3(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [rest]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [t]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "a": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [a]: any
// CHECK-NEXT:  %9 = AllocObjectInst (:object) 1: number, null: null
// CHECK-NEXT:        StoreNewOwnPropertyInst 0: number, %9: object, "a": string, true: boolean
// CHECK-NEXT:  %11 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %12 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: object, %6: any, %9: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: any, [rest]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f4(o: any, t: any): any
// CHECK-NEXT:frame = [o: any, t: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f4(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [o]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [t]: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "a": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: any, [a]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [o]: any
// CHECK-NEXT:  %11 = AllocObjectInst (:object) 1: number, null: null
// CHECK-NEXT:        StoreNewOwnPropertyInst 0: number, %11: object, "a": string, true: boolean
// CHECK-NEXT:  %13 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %14 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %13: object, %7: any, %11: object
// CHECK-NEXT:        StorePropertyLooseInst %14: any, %10: any, "rest": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f5(o: any): any
// CHECK-NEXT:frame = [o: any, a: any, rest: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %f5(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [o]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [rest]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [o]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "a": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [a]: any
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %6: any, "a": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: any, [a]: any
// CHECK-NEXT:  %11 = AllocObjectInst (:object) 1: number, null: null
// CHECK-NEXT:        StoreNewOwnPropertyInst 0: number, %11: object, "a": string, true: boolean
// CHECK-NEXT:  %13 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %14 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %13: object, %6: any, %11: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: any, [rest]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
