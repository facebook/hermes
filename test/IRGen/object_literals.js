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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simpleObjectLiteral": string
// CHECK-NEXT:       DeclareGlobalVarInst "emitAllocObjectLiteral": string
// CHECK-NEXT:       DeclareGlobalVarInst "nestedAllocObjectLiteral": string
// CHECK-NEXT:       DeclareGlobalVarInst "duplicatedObjectLiteral": string
// CHECK-NEXT:       DeclareGlobalVarInst "emptyObjectLiteral": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoObjectLiteral1": string
// CHECK-NEXT:       DeclareGlobalVarInst "protoObjectLiteral2": string
// CHECK-NEXT:       DeclareGlobalVarInst "computedObjectLiteral": string
// CHECK-NEXT:       DeclareGlobalVarInst "spreadObjectLiteral": string
// CHECK-NEXT:        DeclareGlobalVarInst "accessorObjectLiteral1": string
// CHECK-NEXT:        DeclareGlobalVarInst "accessorObjectLiteral2": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %simpleObjectLiteral(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "simpleObjectLiteral": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %emitAllocObjectLiteral(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "emitAllocObjectLiteral": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %0: environment, %nestedAllocObjectLiteral(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "nestedAllocObjectLiteral": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %0: environment, %duplicatedObjectLiteral(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "duplicatedObjectLiteral": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %0: environment, %emptyObjectLiteral(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "emptyObjectLiteral": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %0: environment, %protoObjectLiteral1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "protoObjectLiteral1": string
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %0: environment, %protoObjectLiteral2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %24: object, globalObject: object, "protoObjectLiteral2": string
// CHECK-NEXT:  %26 = CreateFunctionInst (:object) %0: environment, %computedObjectLiteral(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %26: object, globalObject: object, "computedObjectLiteral": string
// CHECK-NEXT:  %28 = CreateFunctionInst (:object) %0: environment, %spreadObjectLiteral(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %28: object, globalObject: object, "spreadObjectLiteral": string
// CHECK-NEXT:  %30 = CreateFunctionInst (:object) %0: environment, %accessorObjectLiteral1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %30: object, globalObject: object, "accessorObjectLiteral1": string
// CHECK-NEXT:  %32 = CreateFunctionInst (:object) %0: environment, %accessorObjectLiteral2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %32: object, globalObject: object, "accessorObjectLiteral2": string
// CHECK-NEXT:  %34 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %34: any
// CHECK-NEXT:  %36 = LoadStackInst (:any) %34: any
// CHECK-NEXT:        ReturnInst %36: any
// CHECK-NEXT:function_end

// CHECK:function simpleObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simpleObjectLiteral(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 10: number, %4: object, "prop1": string, true: boolean
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 10: number, %6: object, "prop1": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function emitAllocObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %emitAllocObjectLiteral(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 6: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 1: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %4: object, "b": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 3: number, %4: object, "c": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 4: number, %4: object, "d": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 5: number, %4: object, "5": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst 6: number, %4: object, "6": string, true: boolean
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function nestedAllocObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %nestedAllocObjectLiteral(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 10: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 100: number, %6: object, "1": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 200: number, %6: object, "2": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst %6: object, %4: object, "b": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst "hello": string, %4: object, "c": string, true: boolean
// CHECK-NEXT:        StoreNewOwnPropertyInst null: null, %4: object, "d": string, true: boolean
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function duplicatedObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %duplicatedObjectLiteral(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 1: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %4: object, "b": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst null: null, %4: object, "d": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 3: number, %4: object, "c": string, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst 4: number, %4: object, "d": string, true: boolean
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function emptyObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %emptyObjectLiteral(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function protoObjectLiteral1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoObjectLiteral1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 3: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 1: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %4: object, "b": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 3: number, %4: object, "c": string, true: boolean
// CHECK-NEXT:  %8 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %9 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object, %8: object
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function protoObjectLiteral2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %protoObjectLiteral2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %5 = AllocObjectInst (:object) 3: number, %4: object
// CHECK-NEXT:       StoreNewOwnPropertyInst 1: number, %5: object, "a": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %5: object, "b": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 3: number, %5: object, "c": string, true: boolean
// CHECK-NEXT:       ReturnInst %5: object
// CHECK-NEXT:function_end

// CHECK:function computedObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %computedObjectLiteral(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 1: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 2: number, %4: object, "b": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 3: number, %4: object, "c": string, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst 4: number, %4: object, "test": string, true: boolean
// CHECK-NEXT:       ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function spreadObjectLiteral(func: any): any
// CHECK-NEXT:frame = [func: any, obj: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %spreadObjectLiteral(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [obj]: any
// CHECK-NEXT:  %5 = AllocObjectInst (:object) 2: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 10: number, %5: object, "a": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst 20: number, %5: object, "b": string, true: boolean
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [obj]: any
// CHECK-NEXT:  %9 = AllocObjectInst (:object) 1: number, empty: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [obj]: any
// CHECK-NEXT:  %11 = CallBuiltinInst (:any) [HermesBuiltin.copyDataProperties]: number, empty: any, empty: any, undefined: undefined, undefined: undefined, %9: object, %10: any
// CHECK-NEXT:        StoreOwnPropertyInst 42: number, %9: object, "c": string, true: boolean
// CHECK-NEXT:        ReturnInst %9: object
// CHECK-NEXT:function_end

// CHECK:function accessorObjectLiteral1(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %accessorObjectLiteral1(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 10: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst "test-str": string, %4: object, "b": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst null: null, %4: object, "c": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst null: null, %4: object, "d": string, true: boolean
// CHECK-NEXT:       StoreOwnPropertyInst 10086: number, %4: object, "c": string, true: boolean
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function accessorObjectLiteral2(func: any): any
// CHECK-NEXT:frame = [func: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %accessorObjectLiteral2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %func: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [func]: any
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 4: number, empty: any
// CHECK-NEXT:       StoreNewOwnPropertyInst 10: number, %4: object, "a": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst "test-str": string, %4: object, "b": string, true: boolean
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %"get c"(): functionCode
// CHECK-NEXT:       StoreGetterSetterInst %7: object, undefined: undefined, %4: object, "c": string, true: boolean
// CHECK-NEXT:       StoreNewOwnPropertyInst null: null, %4: object, "d": string, true: boolean
// CHECK-NEXT:        ReturnInst %4: object
// CHECK-NEXT:function_end

// CHECK:function "get c"(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %accessorObjectLiteral2(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"get c"(): any, %0: environment
// CHECK-NEXT:       ReturnInst 42: number
// CHECK-NEXT:function_end
