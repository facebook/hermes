/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Ensure that __proto__ is not set as an "own" property.

// __proto__ is first property so it can be used as a parent.
function protoIsFirst(func) {
  return {__proto__: func(), a: 2, b: 3};
}

// __proto__ is a constant so it can be used as a parent.
function protoIsConst1() {
  return {a: 2, __proto__: null};
}

function protoIsConst2() {
  return {b: 3, __proto__: 10};
}

function protoIsConst3() {
  return {__proto__: null, c: 4};
}

// We must set it dynamically.
function protoIsDynamic(func, getParent) {
  return {a: func(), b: 10, __proto__: getParent()};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [protoIsFirst, protoIsConst1, protoIsConst2, protoIsConst3, protoIsDynamic]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %protoIsFirst#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "protoIsFirst" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %protoIsConst1#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "protoIsConst1" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %protoIsConst2#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "protoIsConst2" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %protoIsConst3#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "protoIsConst3" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %protoIsDynamic#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "protoIsDynamic" : string
// CHECK-NEXT:  %11 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %12 = StoreStackInst undefined : undefined, %11
// CHECK-NEXT:  %13 = LoadStackInst %11
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:function_end

// CHECK:function protoIsFirst#0#1(func)#2
// CHECK-NEXT:frame = [func#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoIsFirst#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#2], %0
// CHECK-NEXT:  %2 = LoadFrameInst [func#2], %0
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, %3
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 2 : number, %4 : object, "a" : string, true : boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 3 : number, %4 : object, "b" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsConst1#0#1()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoIsConst1#0#1()#3}
// CHECK-NEXT:  %1 = AllocObjectInst 1 : number, null : null
// CHECK-NEXT:  %2 = StoreNewOwnPropertyInst 2 : number, %1 : object, "a" : string, true : boolean
// CHECK-NEXT:  %3 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsConst2#0#1()#4
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoIsConst2#0#1()#4}
// CHECK-NEXT:  %1 = AllocObjectInst 1 : number, 10 : number
// CHECK-NEXT:  %2 = StoreNewOwnPropertyInst 3 : number, %1 : object, "b" : string, true : boolean
// CHECK-NEXT:  %3 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsConst3#0#1()#5
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoIsConst3#0#1()#5}
// CHECK-NEXT:  %1 = AllocObjectInst 1 : number, null : null
// CHECK-NEXT:  %2 = StoreNewOwnPropertyInst 4 : number, %1 : object, "c" : string, true : boolean
// CHECK-NEXT:  %3 = ReturnInst %1 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsDynamic#0#1(func, getParent)#6
// CHECK-NEXT:frame = [func#6, getParent#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{protoIsDynamic#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst %func, [func#6], %0
// CHECK-NEXT:  %2 = StoreFrameInst %getParent, [getParent#6], %0
// CHECK-NEXT:  %3 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %4 = LoadFrameInst [func#6], %0
// CHECK-NEXT:  %5 = CallInst %4, undefined : undefined
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst %5, %3 : object, "a" : string, true : boolean
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst 10 : number, %3 : object, "b" : string, true : boolean
// CHECK-NEXT:  %8 = LoadFrameInst [getParent#6], %0
// CHECK-NEXT:  %9 = CallInst %8, undefined : undefined
// CHECK-NEXT:  %10 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %3 : object, %9
// CHECK-NEXT:  %11 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
