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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [simpleObjectLiteral, emitAllocObjectLiteral, nestedAllocObjectLiteral, duplicatedObjectLiteral, emptyObjectLiteral, protoObjectLiteral1, protoObjectLiteral2, computedObjectLiteral, spreadObjectLiteral, accessorObjectLiteral1, accessorObjectLiteral2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %simpleObjectLiteral#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "simpleObjectLiteral" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %emitAllocObjectLiteral#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "emitAllocObjectLiteral" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %nestedAllocObjectLiteral#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "nestedAllocObjectLiteral" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %duplicatedObjectLiteral#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "duplicatedObjectLiteral" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %emptyObjectLiteral#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "emptyObjectLiteral" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %protoObjectLiteral1#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "protoObjectLiteral1" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %protoObjectLiteral2#0#1()#8, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "protoObjectLiteral2" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %computedObjectLiteral#0#1()#9, %0
// CHECK-NEXT:  %16 = StorePropertyInst %15 : closure, globalObject : object, "computedObjectLiteral" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %spreadObjectLiteral#0#1()#10, %0
// CHECK-NEXT:  %18 = StorePropertyInst %17 : closure, globalObject : object, "spreadObjectLiteral" : string
// CHECK-NEXT:  %19 = CreateFunctionInst %accessorObjectLiteral1#0#1()#11, %0
// CHECK-NEXT:  %20 = StorePropertyInst %19 : closure, globalObject : object, "accessorObjectLiteral1" : string
// CHECK-NEXT:  %21 = CreateFunctionInst %accessorObjectLiteral2#0#1()#12, %0
// CHECK-NEXT:  %22 = StorePropertyInst %21 : closure, globalObject : object, "accessorObjectLiteral2" : string
// CHECK-NEXT:  %23 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %24 = StoreStackInst undefined : undefined, %23
// CHECK-NEXT:  %25 = LoadStackInst %23
// CHECK-NEXT:  %26 = ReturnInst %25
// CHECK-NEXT:function_end

// CHECK:function simpleObjectLiteral#0#1(func)#2
// CHECK-NEXT:frame = [func#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simpleObjectLiteral#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#2], %0
// CHECK-NEXT:  %2 = AllocObjectLiteralInst "prop1" : string, 10 : number
// CHECK-NEXT:  %3 = AllocObjectLiteralInst "prop1" : string, 10 : number
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function emitAllocObjectLiteral#0#1(func)#3
// CHECK-NEXT:frame = [func#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{emitAllocObjectLiteral#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#3], %0
// CHECK-NEXT:  %2 = AllocObjectLiteralInst "a" : string, 1 : number, "b" : string, 2 : number, "c" : string, 3 : number, "d" : string, 4 : number, "5" : string, 5 : number, "6" : string, 6 : number
// CHECK-NEXT:  %3 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function nestedAllocObjectLiteral#0#1(func)#4
// CHECK-NEXT:frame = [func#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{nestedAllocObjectLiteral#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#4], %0
// CHECK-NEXT:  %2 = AllocObjectLiteralInst "1" : string, 100 : number, "2" : string, 200 : number
// CHECK-NEXT:  %3 = AllocObjectLiteralInst "a" : string, 10 : number, "b" : string, %2 : object, "c" : string, "hello" : string, "d" : string, null : null
// CHECK-NEXT:  %4 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function duplicatedObjectLiteral#0#1(func)#5
// CHECK-NEXT:frame = [func#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{duplicatedObjectLiteral#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#5], %0
// CHECK-NEXT:  %2 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 1 : number, %2 : object, "a" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2 : number, %2 : object, "b" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst null : null, %2 : object, "d" : string, true : boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 3 : number, %2 : object, "c" : string, true : boolean
// CHECK-NEXT:  %7 = StoreOwnPropertyInst 4 : number, %2 : object, "d" : string, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function emptyObjectLiteral#0#1(func)#6
// CHECK-NEXT:frame = [func#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{emptyObjectLiteral#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#6], %0
// CHECK-NEXT:  %2 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %3 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoObjectLiteral1#0#1(func)#7
// CHECK-NEXT:frame = [func#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoObjectLiteral1#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#7], %0
// CHECK-NEXT:  %2 = AllocObjectInst 3 : number, empty
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 1 : number, %2 : object, "a" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2 : number, %2 : object, "b" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 3 : number, %2 : object, "c" : string, true : boolean
// CHECK-NEXT:  %6 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %2 : object, %6 : object
// CHECK-NEXT:  %8 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoObjectLiteral2#0#1(func)#8
// CHECK-NEXT:frame = [func#8]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoObjectLiteral2#0#1()#8}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#8], %0
// CHECK-NEXT:  %2 = AllocObjectInst 0 : number, empty
// CHECK-NEXT:  %3 = AllocObjectInst 3 : number, %2 : object
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 1 : number, %3 : object, "a" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 2 : number, %3 : object, "b" : string, true : boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 3 : number, %3 : object, "c" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function computedObjectLiteral#0#1(func)#9
// CHECK-NEXT:frame = [func#9]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{computedObjectLiteral#0#1()#9}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#9], %0
// CHECK-NEXT:  %2 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 1 : number, %2 : object, "a" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2 : number, %2 : object, "b" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 3 : number, %2 : object, "c" : string, true : boolean
// CHECK-NEXT:  %6 = StoreOwnPropertyInst 4 : number, %2 : object, "test" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function spreadObjectLiteral#0#1(func)#10
// CHECK-NEXT:frame = [func#10, obj#10]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{spreadObjectLiteral#0#1()#10}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#10], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [obj#10], %0
// CHECK-NEXT:  %3 = AllocObjectLiteralInst "a" : string, 10 : number, "b" : string, 20 : number
// CHECK-NEXT:  %4 = StoreFrameInst %3 : object, [obj#10], %0
// CHECK-NEXT:  %5 = AllocObjectInst 1 : number, empty
// CHECK-NEXT:  %6 = LoadFrameInst [obj#10], %0
// CHECK-NEXT:  %7 = CallBuiltinInst [HermesBuiltin.copyDataProperties] : number, undefined : undefined, %5 : object, %6
// CHECK-NEXT:  %8 = StoreOwnPropertyInst 42 : number, %5 : object, "c" : string, true : boolean
// CHECK-NEXT:  %9 = ReturnInst %5 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function accessorObjectLiteral1#0#1(func)#11
// CHECK-NEXT:frame = [func#11]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{accessorObjectLiteral1#0#1()#11}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#11], %0
// CHECK-NEXT:  %2 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 10 : number, %2 : object, "a" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst "test-str" : string, %2 : object, "b" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst null : null, %2 : object, "c" : string, true : boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst null : null, %2 : object, "d" : string, true : boolean
// CHECK-NEXT:  %7 = StoreOwnPropertyInst 10086 : number, %2 : object, "c" : string, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function accessorObjectLiteral2#0#1(func)#12
// CHECK-NEXT:frame = [func#12]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{accessorObjectLiteral2#0#1()#12}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#12], %0
// CHECK-NEXT:  %2 = AllocObjectInst 4 : number, empty
// CHECK-NEXT:  %3 = StoreNewOwnPropertyInst 10 : number, %2 : object, "a" : string, true : boolean
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst "test-str" : string, %2 : object, "b" : string, true : boolean
// CHECK-NEXT:  %5 = CreateFunctionInst %"get c"#1#12()#13, %0
// CHECK-NEXT:  %6 = StoreGetterSetterInst %5 : closure, undefined : undefined, %2 : object, "c" : string, true : boolean
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst null : null, %2 : object, "d" : string, true : boolean
// CHECK-NEXT:  %8 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function "get c"#1#12()#13
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{"get c"#1#12()#13}
// CHECK-NEXT:  %1 = ReturnInst 42 : number
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
