/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [simpleObjectLiteral, emitAllocObjectLiteral, nestedAllocObjectLiteral, duplicatedObjectLiteral, emptyObjectLiteral, protoObjectLiteral1, protoObjectLiteral2, computedObjectLiteral, spreadObjectLiteral, accessorObjectLiteral1, accessorObjectLiteral2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %simpleObjectLiteral()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "simpleObjectLiteral" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %emitAllocObjectLiteral()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "emitAllocObjectLiteral" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %nestedAllocObjectLiteral()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "nestedAllocObjectLiteral" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %duplicatedObjectLiteral()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "duplicatedObjectLiteral" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %emptyObjectLiteral()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "emptyObjectLiteral" : string
// CHECK-NEXT:  %10 = CreateFunctionInst %protoObjectLiteral1()
// CHECK-NEXT:  %11 = StorePropertyLooseInst %10 : closure, globalObject : object, "protoObjectLiteral1" : string
// CHECK-NEXT:  %12 = CreateFunctionInst %protoObjectLiteral2()
// CHECK-NEXT:  %13 = StorePropertyLooseInst %12 : closure, globalObject : object, "protoObjectLiteral2" : string
// CHECK-NEXT:  %14 = CreateFunctionInst %computedObjectLiteral()
// CHECK-NEXT:  %15 = StorePropertyLooseInst %14 : closure, globalObject : object, "computedObjectLiteral" : string
// CHECK-NEXT:  %16 = CreateFunctionInst %spreadObjectLiteral()
// CHECK-NEXT:  %17 = StorePropertyLooseInst %16 : closure, globalObject : object, "spreadObjectLiteral" : string
// CHECK-NEXT:  %18 = CreateFunctionInst %accessorObjectLiteral1()
// CHECK-NEXT:  %19 = StorePropertyLooseInst %18 : closure, globalObject : object, "accessorObjectLiteral1" : string
// CHECK-NEXT:  %20 = CreateFunctionInst %accessorObjectLiteral2()
// CHECK-NEXT:  %21 = StorePropertyLooseInst %20 : closure, globalObject : object, "accessorObjectLiteral2" : string
// CHECK-NEXT:  %22 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %23 = StoreStackInst undefined : undefined, %22
// CHECK-NEXT:  %24 = LoadStackInst %22
// CHECK-NEXT:  %25 = ReturnInst %24
// CHECK-NEXT:function_end

// CHECK:function simpleObjectLiteral(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectLiteralInst "prop1" : string, 10 : number
// CHECK-NEXT:  %2 = AllocObjectLiteralInst "prop1" : string, 10 : number
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function emitAllocObjectLiteral(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectLiteralInst "a" : string, 1 : number, "b" : string, 2 : number, "c" : string, 3 : number, "d" : string, 4 : number, "5" : string, 5 : number, "6" : string, 6 : number
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function nestedAllocObjectLiteral(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectLiteralInst "1" : string, 100 : number, "2" : string, 200 : number
// CHECK-NEXT:  %2 = AllocObjectLiteralInst "a" : string, 10 : number, "b" : string, %1 : object, "c" : string, "hello" : string, "d" : string, null : null
// CHECK-NEXT:  %3 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function duplicatedObjectLiteral(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:  %2 = StoreNewOwnPropertyInst 1 : number, %1 : object, "a" : string, true : boolean
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 2 : number, %1 : object, "b" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst null : null, %1 : object, "d" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 3 : number, %1 : object, "c" : string, true : boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst 4 : number, %1 : object, "d" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function emptyObjectLiteral(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %2 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoObjectLiteral1(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectInst 3 : number, empty
// CHECK-NEXT:  %2 = StoreNewOwnPropertyInst 1 : number, %1 : object, "a" : string, true : boolean
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 2 : number, %1 : object, "b" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 3 : number, %1 : object, "c" : string, true : boolean
// CHECK-NEXT:  %5 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %6 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %1 : object, %5 : object
// CHECK-NEXT:  %7 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoObjectLiteral2(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %2 = AllocObjectInst 3 : number, %1 : object
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 1 : number, %2 : object, "a" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2 : number, %2 : object, "b" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 3 : number, %2 : object, "c" : string, true : boolean
// CHECK-NEXT:  %6 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function computedObjectLiteral(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:  %2 = StoreNewOwnPropertyInst 1 : number, %1 : object, "a" : string, true : boolean
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 2 : number, %1 : object, "b" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 3 : number, %1 : object, "c" : string, true : boolean
// CHECK-NEXT:  %5 = StoreOwnPropertyInst 4 : number, %1 : object, "test" : string, true : boolean
// CHECK-NEXT:  %6 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function spreadObjectLiteral(func)
// CHECK-NEXT:frame = [obj, func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [obj]
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %2 = AllocObjectLiteralInst "a" : string, 10 : number, "b" : string, 20 : number
// CHECK-NEXT:  %3 = StoreFrameInst %2 : object, [obj]
// CHECK-NEXT:  %4 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %5 = LoadFrameInst [obj]
// CHECK-NEXT:  %6 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %4 : object, %5
// CHECK-NEXT:  %7 = StoreOwnPropertyInst 42 : number, %4 : object, "c" : string, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function accessorObjectLiteral1(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:  %2 = StoreNewOwnPropertyInst 10 : number, %1 : object, "a" : string, true : boolean
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst "test-str" : string, %1 : object, "b" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst null : null, %1 : object, "c" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst null : null, %1 : object, "d" : string, true : boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst 10086 : number, %1 : object, "c" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function accessorObjectLiteral2(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:  %2 = StoreNewOwnPropertyInst 10 : number, %1 : object, "a" : string, true : boolean
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst "test-str" : string, %1 : object, "b" : string, true : boolean
// CHECK-NEXT:  %4 = CreateFunctionInst %"get c"()
// CHECK-NEXT:  %5 = StoreGetterSetterInst %4 : closure, undefined : undefined, %1 : object, "c" : string, true : boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst null : null, %1 : object, "d" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "get c"()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 42 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
