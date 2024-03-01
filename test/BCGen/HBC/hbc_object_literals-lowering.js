/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes --target=HBC -dump-lir -O %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=IRGEN

// LowerNumericProperties should handle AllocObjectLiteral.
function emitAllocObjectLiteral(func) {
  return {a: 1, b: 2, c: 3, d: 4, 5: 5, '6': 6};
}

// Nested objects are recursively handled.
function nestedAllocObjectLiteral(func) {
  return {a: 10, b:{1: 100, 2:200}, c: "hello", d: null};
}

// Numeric keys do not need placeholder
function numericPlaceholder(func) {
  return {a: 10, 42:{1: 100, 2:200}, c: "hello", d: null};
}

// Lowering continues after reaching estimated best num of elements.
function estimateBestNumElement(func) {
  return { a: 1,
           b: 1,
           c: 1,
           d: 1,
           e: 1,
           f: undefined,
           g: undefined,
           h: 1,
           i: 1,
           j: 1,
           k: 1,
           l: undefined,
           m: undefined,
           n: undefined,
           1: 42,
           2: 42 };
}

// Object literals with accessors can still be partially handled by
// LowerAllocObject pass.
function accessorObjectLiteral(func) {
  return {a: 10, b: "test-str", get c() {return 42;}, d: null};
}

// Auto-generated content below. Please do not modify manually.

// IRGEN:function global(): undefined
// IRGEN-NEXT:frame = []
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// IRGEN-NEXT:       DeclareGlobalVarInst "emitAllocObjectLiteral": string
// IRGEN-NEXT:       DeclareGlobalVarInst "nestedAllocObjectLiteral": string
// IRGEN-NEXT:       DeclareGlobalVarInst "numericPlaceholder": string
// IRGEN-NEXT:       DeclareGlobalVarInst "estimateBestNumElement": string
// IRGEN-NEXT:       DeclareGlobalVarInst "accessorObjectLiteral": string
// IRGEN-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %emitAllocObjectLiteral(): functionCode
// IRGEN-NEXT:  %7 = HBCGetGlobalObjectInst (:object)
// IRGEN-NEXT:       StorePropertyLooseInst %6: object, %7: object, "emitAllocObjectLiteral": string
// IRGEN-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %nestedAllocObjectLiteral(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %9: object, %7: object, "nestedAllocObjectLiteral": string
// IRGEN-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %numericPlaceholder(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %11: object, %7: object, "numericPlaceholder": string
// IRGEN-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %estimateBestNumElement(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %13: object, %7: object, "estimateBestNumElement": string
// IRGEN-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %accessorObjectLiteral(): functionCode
// IRGEN-NEXT:        StorePropertyLooseInst %15: object, %7: object, "accessorObjectLiteral": string
// IRGEN-NEXT:  %17 = HBCLoadConstInst (:undefined) undefined: undefined
// IRGEN-NEXT:        ReturnInst %17: undefined
// IRGEN-NEXT:function_end

// IRGEN:function emitAllocObjectLiteral(func: any): object
// IRGEN-NEXT:frame = []
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst (:object) 6: number, "a": string, 1: number, "b": string, 2: number, "c": string, 3: number, "d": string, 4: number, 5: number, 5: number, 6: number, 6: number
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function nestedAllocObjectLiteral(func: any): object
// IRGEN-NEXT:frame = []
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst (:object) 4: number, "a": string, 10: number, "b": string, null: null, "c": string, "hello": string, "d": string, null: null
// IRGEN-NEXT:  %1 = HBCAllocObjectFromBufferInst (:object) 2: number, 1: number, 100: number, 2: number, 200: number
// IRGEN-NEXT:       StorePropertyLooseInst %1: object, %0: object, "b": string
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function numericPlaceholder(func: any): object
// IRGEN-NEXT:frame = []
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst (:object) 4: number, "a": string, 10: number, "c": string, "hello": string, "d": string, null: null
// IRGEN-NEXT:  %1 = HBCAllocObjectFromBufferInst (:object) 2: number, 1: number, 100: number, 2: number, 200: number
// IRGEN-NEXT:       StoreNewOwnPropertyInst %1: object, %0: object, 42: number, true: boolean
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function estimateBestNumElement(func: any): object
// IRGEN-NEXT:frame = []
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst (:object) 16: number, "a": string, 1: number, "b": string, 1: number, "c": string, 1: number, "d": string, 1: number, "e": string, 1: number, "f": string, null: null, "g": string, null: null, "h": string, 1: number, "i": string, 1: number, "j": string, 1: number, "k": string, 1: number
// IRGEN-NEXT:  %1 = HBCLoadConstInst (:undefined) undefined: undefined
// IRGEN-NEXT:       StorePropertyLooseInst %1: undefined, %0: object, "f": string
// IRGEN-NEXT:       StorePropertyLooseInst %1: undefined, %0: object, "g": string
// IRGEN-NEXT:       StoreNewOwnPropertyInst %1: undefined, %0: object, "l": string, true: boolean
// IRGEN-NEXT:       StoreNewOwnPropertyInst %1: undefined, %0: object, "m": string, true: boolean
// IRGEN-NEXT:       StoreNewOwnPropertyInst %1: undefined, %0: object, "n": string, true: boolean
// IRGEN-NEXT:  %7 = HBCLoadConstInst (:number) 42: number
// IRGEN-NEXT:       StoreNewOwnPropertyInst %7: number, %0: object, 1: number, true: boolean
// IRGEN-NEXT:       StoreNewOwnPropertyInst %7: number, %0: object, 2: number, true: boolean
// IRGEN-NEXT:        ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function accessorObjectLiteral(func: any): object
// IRGEN-NEXT:frame = []
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst (:object) 4: number, "a": string, 10: number, "b": string, "test-str": string
// IRGEN-NEXT:  %1 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// IRGEN-NEXT:  %2 = CreateScopeInst (:environment) %accessorObjectLiteral(): any, %1: environment
// IRGEN-NEXT:  %3 = CreateFunctionInst (:object) %2: environment, %"get c"(): functionCode
// IRGEN-NEXT:  %4 = HBCLoadConstInst (:undefined) undefined: undefined
// IRGEN-NEXT:  %5 = HBCLoadConstInst (:string) "c": string
// IRGEN-NEXT:       StoreGetterSetterInst %3: object, %4: undefined, %0: object, %5: string, true: boolean
// IRGEN-NEXT:  %7 = HBCLoadConstInst (:null) null: null
// IRGEN-NEXT:       StoreNewOwnPropertyInst %7: null, %0: object, "d": string, true: boolean
// IRGEN-NEXT:       ReturnInst %0: object
// IRGEN-NEXT:function_end

// IRGEN:function "get c"(): number
// IRGEN-NEXT:frame = []
// IRGEN-NEXT:%BB0:
// IRGEN-NEXT:  %0 = HBCLoadConstInst (:number) 42: number
// IRGEN-NEXT:       ReturnInst %0: number
// IRGEN-NEXT:function_end
