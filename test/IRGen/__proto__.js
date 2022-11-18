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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [protoIsFirst, protoIsConst1, protoIsConst2, protoIsConst3, protoIsDynamic]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %protoIsFirst()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "protoIsFirst" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %protoIsConst1()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "protoIsConst1" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %protoIsConst2()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "protoIsConst2" : string
// CHECK-NEXT:  %6 = CreateFunctionInst %protoIsConst3()
// CHECK-NEXT:  %7 = StorePropertyLooseInst %6 : closure, globalObject : object, "protoIsConst3" : string
// CHECK-NEXT:  %8 = CreateFunctionInst %protoIsDynamic()
// CHECK-NEXT:  %9 = StorePropertyLooseInst %8 : closure, globalObject : object, "protoIsDynamic" : string
// CHECK-NEXT:  %10 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %11 = StoreStackInst undefined : undefined, %10
// CHECK-NEXT:  %12 = LoadStackInst %10
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:function_end

// CHECK:function protoIsFirst(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = LoadFrameInst [func]
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined
// CHECK-NEXT:  %3 = AllocObjectInst 2 : number, %2
// CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2 : number, %3 : object, "a" : string, true : boolean
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 3 : number, %3 : object, "b" : string, true : boolean
// CHECK-NEXT:  %6 = ReturnInst %3 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsConst1()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectInst 1 : number, null : null
// CHECK-NEXT:  %1 = StoreNewOwnPropertyInst 2 : number, %0 : object, "a" : string, true : boolean
// CHECK-NEXT:  %2 = ReturnInst %0 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsConst2()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectInst 1 : number, 10 : number
// CHECK-NEXT:  %1 = StoreNewOwnPropertyInst 3 : number, %0 : object, "b" : string, true : boolean
// CHECK-NEXT:  %2 = ReturnInst %0 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsConst3()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectInst 1 : number, null : null
// CHECK-NEXT:  %1 = StoreNewOwnPropertyInst 4 : number, %0 : object, "c" : string, true : boolean
// CHECK-NEXT:  %2 = ReturnInst %0 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function protoIsDynamic(func, getParent)
// CHECK-NEXT:frame = [func, getParent]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
// CHECK-NEXT:  %1 = StoreFrameInst %getParent, [getParent]
// CHECK-NEXT:  %2 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %3 = LoadFrameInst [func]
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst %4, %2 : object, "a" : string, true : boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 10 : number, %2 : object, "b" : string, true : boolean
// CHECK-NEXT:  %7 = LoadFrameInst [getParent]
// CHECK-NEXT:  %8 = CallInst %7, undefined : undefined
// CHECK-NEXT:  %9 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %2 : object, %8
// CHECK-NEXT:  %10 = ReturnInst %2 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
