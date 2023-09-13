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
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2": string
// CHECK-NEXT:       DeclareGlobalVarInst "f3": string
// CHECK-NEXT:       DeclareGlobalVarInst "f4": string
// CHECK-NEXT:       DeclareGlobalVarInst "f5": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %f1(): any
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "f1": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %f2(): any
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "f2": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %f3(): any
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "f3": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %f4(): any
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "f4": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %f5(): any
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "f5": string
// CHECK-NEXT:  %15 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %15: any
// CHECK-NEXT:  %17 = LoadStackInst (:any) %15: any
// CHECK-NEXT:        ReturnInst %17: any
// CHECK-NEXT:function_end

// CHECK:function f1(t: any): any
// CHECK-NEXT:frame = [t: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %4 = BinaryEqualInst (:any) %3: any, null: null
// CHECK-NEXT:       CondBranchInst %4: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ThrowTypeErrorInst "Cannot destructure 'undefined' or 'null'.": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %7 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %8 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %7: object, %3: any, undefined: undefined
// CHECK-NEXT:       StoreFrameInst %8: any, [a]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [a]: any
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function f2(t: any): any
// CHECK-NEXT:frame = [t: any, a: any, b: any, rest: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [b]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [rest]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "a": string
// CHECK-NEXT:       StoreFrameInst %6: any, [a]: any
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %5: any, "b": string
// CHECK-NEXT:       StoreFrameInst %8: any, [b]: any
// CHECK-NEXT:  %10 = AllocObjectLiteralInst (:object) "a": string, 0: number, "b": string, 0: number
// CHECK-NEXT:  %11 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %10: object, null: null
// CHECK-NEXT:  %12 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %13 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %12: object, %5: any, %10: object
// CHECK-NEXT:        StoreFrameInst %13: any, [rest]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [rest]: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function f3(t: any): any
// CHECK-NEXT:frame = [t: any, a: any, rest: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %0: any, [t]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [rest]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "a": string
// CHECK-NEXT:       StoreFrameInst %5: any, [a]: any
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) "a": string, 0: number
// CHECK-NEXT:  %8 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %7: object, null: null
// CHECK-NEXT:  %9 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %10 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: object, %4: any, %7: object
// CHECK-NEXT:        StoreFrameInst %10: any, [rest]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f4(o: any, t: any): any
// CHECK-NEXT:frame = [o: any, t: any, a: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %0: any, [o]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %t: any
// CHECK-NEXT:       StoreFrameInst %2: any, [t]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [t]: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "a": string
// CHECK-NEXT:       StoreFrameInst %6: any, [a]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %9 = AllocObjectLiteralInst (:object) "a": string, 0: number
// CHECK-NEXT:  %10 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: object, null: null
// CHECK-NEXT:  %11 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %12 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: object, %5: any, %9: object
// CHECK-NEXT:        StorePropertyLooseInst %12: any, %8: any, "rest": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f5(o: any): any
// CHECK-NEXT:frame = [o: any, a: any, rest: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %o: any
// CHECK-NEXT:       StoreFrameInst %0: any, [o]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [a]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [rest]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [o]: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "a": string
// CHECK-NEXT:       StoreFrameInst %5: any, [a]: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %4: any, "a": string
// CHECK-NEXT:       StoreFrameInst %7: any, [a]: any
// CHECK-NEXT:  %9 = AllocObjectLiteralInst (:object) "a": string, 0: number
// CHECK-NEXT:  %10 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: object, null: null
// CHECK-NEXT:  %11 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %12 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %11: object, %4: any, %9: object
// CHECK-NEXT:        StoreFrameInst %12: any, [rest]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
