/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s

// Ensure that __proto__ is not set as an "own" property.

// __proto__ is first property so it can be used as a parent.
function protoIsFirst(func) {
  return {__proto__: func(), a: 2, b: 3};
}
//CHECK-LABEL:function protoIsFirst(func)
//CHECK-NEXT:frame = [func]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
//CHECK-NEXT:  %1 = LoadFrameInst [func]
//CHECK-NEXT:  %2 = CallInst %1, undefined : undefined
//CHECK-NEXT:  %3 = AllocObjectInst 2 : number, %2
//CHECK-NEXT:  %4 = StoreNewOwnPropertyInst 2 : number, %3 : object, "a" : string, true : boolean
//CHECK-NEXT:  %5 = StoreNewOwnPropertyInst 3 : number, %3 : object, "b" : string, true : boolean
//CHECK-NEXT:  %6 = ReturnInst %3 : object


// __proto__ is a constant so it can be used as a parent.
function protoIsConst1() {
  return {a: 2, __proto__: null};
}
//CHECK-LABEL:function protoIsConst1()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocObjectInst 1 : number, null : null
//CHECK-NEXT:  %1 = StoreNewOwnPropertyInst 2 : number, %0 : object, "a" : string, true : boolean
//CHECK-NEXT:  %2 = ReturnInst %0 : object

function protoIsConst2() {
  return {b: 3, __proto__: 10};
}
//CHECK-LABEL:function protoIsConst2()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocObjectInst 1 : number, 10 : number
//CHECK-NEXT:  %1 = StoreNewOwnPropertyInst 3 : number, %0 : object, "b" : string, true : boolean
//CHECK-NEXT:  %2 = ReturnInst %0 : object

function protoIsConst3() {
  return {__proto__: null, c: 4};
}
//CHECK-LABEL:function protoIsConst3()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = AllocObjectInst 1 : number, null : null
//CHECK-NEXT:  %1 = StoreNewOwnPropertyInst 4 : number, %0 : object, "c" : string, true : boolean
//CHECK-NEXT:  %2 = ReturnInst %0 : object

// We must set it dynamically.
function protoIsDynamic(func, getParent) {
  return {a: func(), b: 10, __proto__: getParent()};
}
//CHECK-LABEL:function protoIsDynamic(func, getParent)
//CHECK-NEXT:frame = [func, getParent]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst %func, [func]
//CHECK-NEXT:  %1 = StoreFrameInst %getParent, [getParent]
//CHECK-NEXT:  %2 = AllocObjectInst 2 : number, empty
//CHECK-NEXT:  %3 = LoadFrameInst [func]
//CHECK-NEXT:  %4 = CallInst %3, undefined : undefined
//CHECK-NEXT:  %5 = StoreNewOwnPropertyInst %4, %2 : object, "a" : string, true : boolean
//CHECK-NEXT:  %6 = StoreNewOwnPropertyInst 10 : number, %2 : object, "b" : string, true : boolean
//CHECK-NEXT:  %7 = LoadFrameInst [getParent]
//CHECK-NEXT:  %8 = CallInst %7, undefined : undefined
//CHECK-NEXT:  %9 = CallBuiltinInst [HermesBuiltin.silentSetPrototypeOf] : number, undefined : undefined, %2 : object, %8
//CHECK-NEXT:  %10 = ReturnInst %2 : object
