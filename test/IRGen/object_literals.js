/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function simpleObjectLiteral(func) {
  ({prop1: 10});
  ({"prop1": 10});
}

// Emit AllocObjectLiteral for most object literals.
function emitAllocObjectLiteral(func) {
  return {a: 1, b: 2, c: 3, d: 4, 5: 5, '6': 6};
}

// Nested objects are recursively handled.
function nestedAllocObjectLiteral(func) {
  return {a: 10, b:{1: 100, 2:200}, c: "hello", d: null};
}

// Do not emit AllocObjectLiteral for duplicated properties.
function duplicatedObjectLiteral(func) {
  return {a: 1, b: 2, d: 42, c: 3, d: 4};
}

// Do not emit AllocObjectLiteral if object is empty.
function emptyObjectLiteral(func) {
  return {};
}

// Do not emit AllocObjectLiteral if __proto__ is set.
function protoObjectLiteral1(func) {
  return {a: 1, b: 2, c: 3, __proto__: {}};
}

function protoObjectLiteral2(func) {
  return {__proto__: {}, a: 1, b: 2, c: 3};
}

// Do not emit AllocObjectLiteral if there is computed key.
function computedObjectLiteral(func) {
  return {a: 1, b: 2, c: 3, ['test']: 4};
}

// Do not emit AllocObjectLiteral if there is spread node.
function spreadObjectLiteral(func) {
  var obj = {a: 10, b: 20};
  return {...obj, c: 42};
}

// Overwritten accessors are ok.
function accessorObjectLiteral1(func) {
  return {a: 10, b: "test-str", get c() {return 42;}, d: null, c: 10086};
}

// Do not emit AllocObjectLiteral if there is an accessor.
function accessorObjectLiteral2(func) {
  return {a: 10, b: "test-str", get c() {return 42;}, d: null};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simpleObjectLiteral": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "emitAllocObjectLiteral": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "nestedAllocObjectLiteral": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "duplicatedObjectLiteral": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "emptyObjectLiteral": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "protoObjectLiteral1": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "protoObjectLiteral2": string
// CHECK-NEXT:  %7 = DeclareGlobalVarInst "computedObjectLiteral": string
// CHECK-NEXT:  %8 = DeclareGlobalVarInst "spreadObjectLiteral": string
// CHECK-NEXT:  %9 = DeclareGlobalVarInst "accessorObjectLiteral1": string
// CHECK-NEXT:  %10 = DeclareGlobalVarInst "accessorObjectLiteral2": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %simpleObjectLiteral(): any
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11: object, globalObject: object, "simpleObjectLiteral": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %emitAllocObjectLiteral(): any
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: object, globalObject: object, "emitAllocObjectLiteral": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %nestedAllocObjectLiteral(): any
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15: object, globalObject: object, "nestedAllocObjectLiteral": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %duplicatedObjectLiteral(): any
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17: object, globalObject: object, "duplicatedObjectLiteral": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %emptyObjectLiteral(): any
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19: object, globalObject: object, "emptyObjectLiteral": string
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %protoObjectLiteral1(): any
// CHECK-NEXT:  %22 = StorePropertyLooseInst %21: object, globalObject: object, "protoObjectLiteral1": string
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %protoObjectLiteral2(): any
// CHECK-NEXT:  %24 = StorePropertyLooseInst %23: object, globalObject: object, "protoObjectLiteral2": string
// CHECK-NEXT:  %25 = CreateFunctionInst (:object) %computedObjectLiteral(): any
// CHECK-NEXT:  %26 = StorePropertyLooseInst %25: object, globalObject: object, "computedObjectLiteral": string
// CHECK-NEXT:  %27 = CreateFunctionInst (:object) %spreadObjectLiteral(): any
// CHECK-NEXT:  %28 = StorePropertyLooseInst %27: object, globalObject: object, "spreadObjectLiteral": string
// CHECK-NEXT:  %29 = CreateFunctionInst (:object) %accessorObjectLiteral1(): any
// CHECK-NEXT:  %30 = StorePropertyLooseInst %29: object, globalObject: object, "accessorObjectLiteral1": string
// CHECK-NEXT:  %31 = CreateFunctionInst (:object) %accessorObjectLiteral2(): any
// CHECK-NEXT:  %32 = StorePropertyLooseInst %31: object, globalObject: object, "accessorObjectLiteral2": string
// CHECK-NEXT:  %33 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %34 = StoreStackInst undefined: undefined, %33: any
// CHECK-NEXT:  %35 = LoadStackInst (:any) %33: any
// CHECK-NEXT:  %36 = ReturnInst %35: any
// CHECK-NEXT:function_end

// CHECK:function simpleObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) "prop1": string, 10: number
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) "prop1": string, 10: number
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function emitAllocObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) "a": string, 1: number, "b": string, 2: number, "c": string, 3: number, "d": string, 4: number, "5": string, 5: number, "6": string, 6: number
// CHECK-NEXT:  %3 = ReturnInst %2: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function nestedAllocObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) "1": string, 100: number, "2": string, 200: number
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) "a": string, 10: number, "b": string, %2: object, "c": string, "hello": string, "d": string, null: null
// CHECK-NEXT:  %4 = ReturnInst %3: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function duplicatedObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 1: number, %2: object, "a": string, true: boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2: number, %2: object, "b": string, true: boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst null: null, %2: object, "d": string, true: boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 3: number, %2: object, "c": string, true: boolean
// CHECK-NEXT:  %7 = StoreOwnPropertyInst 4: number, %2: object, "d": string, true: boolean
// CHECK-NEXT:  %8 = ReturnInst %2: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function emptyObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %3 = ReturnInst %2: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoObjectLiteral1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 3: number, empty: any
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 1: number, %2: object, "a": string, true: boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2: number, %2: object, "b": string, true: boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 3: number, %2: object, "c": string, true: boolean
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %7 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: object, %6: object
// CHECK-NEXT:  %8 = ReturnInst %2: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function protoObjectLiteral2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 3: number, %2: object
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 1: number, %3: object, "a": string, true: boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 2: number, %3: object, "b": string, true: boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 3: number, %3: object, "c": string, true: boolean
// CHECK-NEXT:  %7 = ReturnInst %3: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function computedObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 1: number, %2: object, "a": string, true: boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2: number, %2: object, "b": string, true: boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 3: number, %2: object, "c": string, true: boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst 4: number, %2: object, "test": string, true: boolean
// CHECK-NEXT:  %7 = ReturnInst %2: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function spreadObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any, obj: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = StoreFrameInst undefined: undefined, [obj]: any
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) "a": string, 10: number, "b": string, 20: number
// CHECK-NEXT:  %4 = StoreFrameInst %3: object, [obj]: any
// CHECK-NEXT:  %5 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [obj]: any
// CHECK-NEXT:  %7 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %5: object, %6: any
// CHECK-NEXT:  %8 = StoreOwnPropertyInst 42: number, %5: object, "c": string, true: boolean
// CHECK-NEXT:  %9 = ReturnInst %5: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function accessorObjectLiteral1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 10: number, %2: object, "a": string, true: boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst "test-str": string, %2: object, "b": string, true: boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst null: null, %2: object, "c": string, true: boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst null: null, %2: object, "d": string, true: boolean
// CHECK-NEXT:  %7 = StoreOwnPropertyInst 10086: number, %2: object, "c": string, true: boolean
// CHECK-NEXT:  %8 = ReturnInst %2: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function accessorObjectLiteral2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %func: any
// CHECK-NEXT:  %1 = StoreFrameInst %0: any, [func]: any
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 10: number, %2: object, "a": string, true: boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst "test-str": string, %2: object, "b": string, true: boolean
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %"get c"(): any
// CHECK-NEXT:  %6 = StoreGetterSetterInst %5: object, undefined: undefined, %2: object, "c": string, true: boolean
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst null: null, %2: object, "d": string, true: boolean
// CHECK-NEXT:  %8 = ReturnInst %2: object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "get c"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 42: number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
