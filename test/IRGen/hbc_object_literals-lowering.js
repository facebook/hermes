/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes --target=HBC -dump-lir -O %s | %FileCheck %s --match-full-lines --check-prefix=IRGEN

// LowerNumericProperties should handle AllocObjectLiteral.
function emitAllocObjectLiteral(func) {
  return {a: 1, b: 2, c: 3, d: 4, 5: 5, '6': 6};
}
//IRGEN-LABEL:function emitAllocObjectLiteral(func) : object
//IRGEN-NEXT:frame = []
//IRGEN-NEXT:%BB0:
//IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst 6 : number, "a" : string, 1 : number, "b" : string, 2 : number, "c" : string, 3 : number, "d" : string, 4 : number, 5 : number, 5 : number, 6 : number, 6 : number
//IRGEN-NEXT:  %1 = ReturnInst %0 : object
//IRGEN-NEXT:function_end

// Nested objects are recursively handled.
function nestedAllocObjectLiteral(func) {
  return {a: 10, b:{1: 100, 2:200}, c: "hello", d: null};
}
//IRGEN-LABEL:function nestedAllocObjectLiteral(func) : object
//IRGEN-NEXT:frame = []
//IRGEN-NEXT:%BB0:
//IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst 4 : number, "a" : string, 10 : number, "b" : string, null : null, "c" : string, "hello" : string, "d" : string, null : null
//IRGEN-NEXT:  %1 = HBCAllocObjectFromBufferInst 2 : number, 1 : number, 100 : number, 2 : number, 200 : number
//IRGEN-NEXT:  %2 = StorePropertyInst %1 : object, %0 : object, "b" : string
//IRGEN-NEXT:  %3 = ReturnInst %0 : object
//IRGEN-NEXT:function_end

// Numeric keys do not need placeholder
function numericPlaceholder(func) {
  return {a: 10, 42:{1: 100, 2:200}, c: "hello", d: null};
}
//IRGEN-LABEL:function numericPlaceholder(func) : object
//IRGEN-NEXT:frame = []
//IRGEN-NEXT:%BB0:
//IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst 4 : number, "a" : string, 10 : number, "c" : string, "hello" : string, "d" : string, null : null
//IRGEN-NEXT:  %1 = HBCAllocObjectFromBufferInst 2 : number, 1 : number, 100 : number, 2 : number, 200 : number
//IRGEN-NEXT:  %2 = StoreOwnPropertyInst %1 : object, %0 : object, 42 : number, true : boolean
//IRGEN-NEXT:  %3 = ReturnInst %0 : object
//IRGEN-NEXT:function_end

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
//IRGEN-LABEL:function estimateBestNumElement(func) : object
//IRGEN-NEXT:frame = []
//IRGEN-NEXT:%BB0:
//IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst 16 : number, "a" : string, 1 : number, "b" : string, 1 : number, "c" : string, 1 : number, "d" : string, 1 : number, "e" : string, 1 : number, "f" : string, null : null, "g" : string, null : null, "h" : string, 1 : number, "i" : string, 1 : number, "j" : string, 1 : number, "k" : string, 1 : number
//IRGEN-NEXT:  %1 = HBCLoadConstInst undefined : undefined
//IRGEN-NEXT:  %2 = StorePropertyInst %1 : undefined, %0 : object, "f" : string
//IRGEN-NEXT:  %3 = StorePropertyInst %1 : undefined, %0 : object, "g" : string
//IRGEN-NEXT:  %4 = StoreNewOwnPropertyInst %1 : undefined, %0 : object, "l" : string, true : boolean
//IRGEN-NEXT:  %5 = StoreNewOwnPropertyInst %1 : undefined, %0 : object, "m" : string, true : boolean
//IRGEN-NEXT:  %6 = StoreNewOwnPropertyInst %1 : undefined, %0 : object, "n" : string, true : boolean
//IRGEN-NEXT:  %7 = HBCLoadConstInst 42 : number
//IRGEN-NEXT:  %8 = StoreNewOwnPropertyInst %7 : number, %0 : object, 1 : number, true : boolean
//IRGEN-NEXT:  %9 = StoreNewOwnPropertyInst %7 : number, %0 : object, 2 : number, true : boolean
//IRGEN-NEXT:  %10 = ReturnInst %0 : object
//IRGEN-NEXT:function_end

// Object literals with accessors can still be partially handled by
// LowerAllocObject pass.
function accessorObjectLiteral(func) {
  return {a: 10, b: "test-str", get c() {return 42;}, d: null};
}
//IRGEN-LABEL:function accessorObjectLiteral(func) : object
//IRGEN-NEXT:frame = []
//IRGEN-NEXT:%BB0:
//IRGEN-NEXT:  %0 = HBCAllocObjectFromBufferInst 4 : number, "a" : string, 10 : number, "b" : string, "test-str" : string
//IRGEN-NEXT:  %1 = HBCCreateEnvironmentInst
//IRGEN-NEXT:  %2 = HBCCreateFunctionInst %"get c"() : number, %1
//IRGEN-NEXT:  %3 = HBCLoadConstInst undefined : undefined
//IRGEN-NEXT:  %4 = HBCLoadConstInst "c" : string
//IRGEN-NEXT:  %5 = StoreGetterSetterInst %2 : closure, %3 : undefined, %0 : object, %4 : string, true : boolean
//IRGEN-NEXT:  %6 = HBCLoadConstInst null : null
//IRGEN-NEXT:  %7 = StoreNewOwnPropertyInst %6 : null, %0 : object, "d" : string, true : boolean
//IRGEN-NEXT:  %8 = ReturnInst %0 : object
//IRGEN-NEXT:function_end
