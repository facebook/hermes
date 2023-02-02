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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "protoIsFirst" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "protoIsConst1" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "protoIsConst2" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "protoIsConst3" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "protoIsDynamic" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %protoIsFirst()
// CHECK-NEXT:  %6 = StorePropertyLooseInst %5 : closure, globalObject : object, "protoIsFirst" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %protoIsConst1()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "protoIsConst1" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %protoIsConst2()
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9 : closure, globalObject : object, "protoIsConst2" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %protoIsConst3()
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11 : closure, globalObject : object, "protoIsConst3" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %protoIsDynamic()
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13 : closure, globalObject : object, "protoIsDynamic" : string
// CHECK-NEXT:  %15 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %16 = StoreStackInst undefined : undefined, %15
// CHECK-NEXT:  %17 = LoadStackInst %15
// CHECK-NEXT:  %18 = ReturnInst %17
// CHECK-NEXT:function_end

// CHECK:function protoIsFirst(func)
// CHECK-NEXT:frame = [func]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %func
// CHECK-NEXT:  %1 = StoreFrameInst %0, [func]
// CHECK-NEXT:  %2 = LoadFrameInst [func]
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, %3
// CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 2 : number, %4 : object, "a" : string, true : boolean
// CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 3 : number, %4 : object, "b" : string, true : boolean
// CHECK-NEXT:  %7 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
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
// CHECK-NEXT:  %0 = LoadParamInst %func
// CHECK-NEXT:  %1 = StoreFrameInst %0, [func]
// CHECK-NEXT:  %2 = LoadParamInst %getParent
// CHECK-NEXT:  %3 = StoreFrameInst %2, [getParent]
// CHECK-NEXT:  %4 = AllocObjectInst 2 : number, empty
// CHECK-NEXT:  %5 = LoadFrameInst [func]
// CHECK-NEXT:  %6 = CallInst %5, undefined : undefined
// CHECK-NEXT:  %7 = StoreNewOwnPropertyInst %6, %4 : object, "a" : string, true : boolean
// CHECK-NEXT:  %8 = StoreNewOwnPropertyInst 10 : number, %4 : object, "b" : string, true : boolean
// CHECK-NEXT:  %9 = LoadFrameInst [getParent]
// CHECK-NEXT:  %10 = CallInst %9, undefined : undefined
// CHECK-NEXT:  %11 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %4 : object, %10
// CHECK-NEXT:  %12 = ReturnInst %4 : object
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
