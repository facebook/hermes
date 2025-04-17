/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ir -O %s | %FileCheckOrRegen --match-full-lines %s

function foo(p) {
  var obj = {a: 0, b: 1};
  return obj;
}

// Check that storing the global object in an object literal generates a
// placeholder. This is a special case because the global object is treated
// as a Literal inside the compiler.
(function () {
  "noinline";
  var o2 = {
    "foo": "fail",
    "hello": this,
    "x" : 5
  };
  return o2;
}());

function inlineComputedKeys(){
  return (function (a,b,c){
    return {[a]: 0, [b]: 1, [c]: 3};
  })("foo", "bar", "baz");
}

function repeatProperty(){
  return {a: 3, b: 5, a: 10, c: null, a: "hello"};
}

function objPhiUser(sink){
  let x;
  while (sink()){
    // Phi on x is inserted at the start here.
    sink(x);
    x = {a: 1, b: 2, c: 3};
  }
}

class MyClass {
  a = 1;
  b = "I am a string";
  c = null;
  // Calling a method causes this to escape.
  d = this.myMethod();
  e = undefined;

  myMethod () {
    return 42;
  }
}
// Force the constructor to escape so it isn't deleted.
globalThis.MyClass = MyClass;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "inlineComputedKeys": string
// CHECK-NEXT:       DeclareGlobalVarInst "repeatProperty": string
// CHECK-NEXT:       DeclareGlobalVarInst "objPhiUser": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) empty: any, empty: any, %inlineComputedKeys(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "inlineComputedKeys": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) empty: any, empty: any, %repeatProperty(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "repeatProperty": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) empty: any, empty: any, %objPhiUser(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "objPhiUser": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) empty: any, empty: any, %""(): functionCode
// CHECK-NEXT:  %13 = CallInst (:object) %12: object, %""(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %14 = AllocStackInst (:object) $?anon_1_clsPrototype: any
// CHECK-NEXT:  %15 = CreateClassInst (:object) empty: any, empty: any, %MyClass(): functionCode, empty: any, %14: object
// CHECK-NEXT:  %16 = LoadStackInst (:object) %14: object
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) empty: any, empty: any, %myMethod(): functionCode
// CHECK-NEXT:        DefineOwnPropertyInst %17: object, %16: object, "myMethod": string, false: boolean
// CHECK-NEXT:  %19 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:        StorePropertyLooseInst %15: object, %19: any, "MyClass": string
// CHECK-NEXT:        ReturnInst %15: object
// CHECK-NEXT:function_end

// CHECK:function foo(p: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectLiteralInst (:object) empty: any, "a": string, 0: number, "b": string, 1: number
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function inlineComputedKeys(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectLiteralInst (:object) empty: any, "foo": string, 0: number, "bar": string, 1: number, "baz": string, 3: number
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function repeatProperty(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocObjectLiteralInst (:object) empty: any, "a": string, "hello": string, "b": string, 5: number, "c": string, null: null
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function objPhiUser(sink: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %sink: any
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       CondBranchInst %1: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = PhiInst (:undefined|object) undefined: undefined, %BB0, %5: object, %BB1
// CHECK-NEXT:  %4 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %3: undefined|object
// CHECK-NEXT:  %5 = AllocObjectLiteralInst (:object) empty: any, "a": string, 1: number, "b": string, 2: number, "c": string, 3: number
// CHECK-NEXT:  %6 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       CondBranchInst %6: any, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) empty: any, "foo": string, "fail": string, "hello": string, null: null, "x": string, 5: number
// CHECK-NEXT:       PrStoreInst %1: object, %2: object, 1: number, "hello": string, false: boolean
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:base constructor MyClass(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) %1: any, "a": string, 1: number, "b": string, "I am a string": string, "c": string, null: null
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "myMethod": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, %2: object
// CHECK-NEXT:       DefineOwnPropertyInst %4: any, %2: object, "d": string, true: boolean
// CHECK-NEXT:       DefineOwnPropertyInst undefined: undefined, %2: object, "e": string, true: boolean
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:method myMethod(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 42: number
// CHECK-NEXT:function_end
