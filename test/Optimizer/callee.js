/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O -fno-inline | %FileCheckOrRegen %s

"use strict";

function fuzz() {

  function foo(k) {
    return k
  }

  return foo(12)
}

function ctor_test() {

  function foo(k) {
    return k
  }

  return new foo(12)
}

function load_store_test() {

  var k = function(k) { return k }

  function ping() {
      return k(123)
  }

  return ping()
}

function load_store_multiple_test() {
  function foo(cond, val) { if(cond) return val }

  function bar() { return foo(true, 7) + foo(true, 8) }

  return bar()
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "fuzz": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "ctor_test": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "load_store_test": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "load_store_multiple_test": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %fuzz(): number
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4: closure, globalObject: object, "fuzz": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %ctor_test(): object
// CHECK-NEXT:  %7 = StorePropertyStrictInst %6: closure, globalObject: object, "ctor_test": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:closure) %load_store_test(): number
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8: closure, globalObject: object, "load_store_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:closure) %load_store_multiple_test(): number
// CHECK-NEXT:  %11 = StorePropertyStrictInst %10: closure, globalObject: object, "load_store_multiple_test": string
// CHECK-NEXT:  %12 = ReturnInst (:string) "use strict": string
// CHECK-NEXT:function_end

// CHECK:function fuzz(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %foo(): number
// CHECK-NEXT:  %1 = CallInst (:number) %0: closure, empty: any, empty: any, undefined: undefined, 12: number
// CHECK-NEXT:  %2 = ReturnInst (:number) 12: number
// CHECK-NEXT:function_end

// CHECK:function ctor_test(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %"foo 1#"(): number
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: closure, "prototype": string
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: any, %0: closure
// CHECK-NEXT:  %3 = ConstructInst (:number) %0: closure, empty: any, empty: any, undefined: undefined, 12: number
// CHECK-NEXT:  %4 = GetConstructedObjectInst (:object) %2: object, %3: number
// CHECK-NEXT:  %5 = ReturnInst (:object) %4: object
// CHECK-NEXT:function_end

// CHECK:function load_store_test(): number
// CHECK-NEXT:frame = [k: closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %ping(): number
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %k(): number
// CHECK-NEXT:  %2 = StoreFrameInst %1: closure, [k]: closure
// CHECK-NEXT:  %3 = CallInst (:number) %0: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %4 = ReturnInst (:number) 123: number
// CHECK-NEXT:function_end

// CHECK:function load_store_multiple_test(): number
// CHECK-NEXT:frame = [foo: closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %"foo 2#"(): number
// CHECK-NEXT:  %1 = StoreFrameInst %0: closure, [foo]: closure
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %bar(): number
// CHECK-NEXT:  %3 = CallInst (:number) %2: closure, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %4 = ReturnInst (:number) %3: number
// CHECK-NEXT:function_end

// CHECK:function foo(k: number): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:number) 12: number
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"(k: number): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:number) 12: number
// CHECK-NEXT:function_end

// CHECK:function ping(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:closure) [k@load_store_test]: closure
// CHECK-NEXT:  %1 = CallInst (:number) %0: closure, empty: any, empty: any, undefined: undefined, 123: number
// CHECK-NEXT:  %2 = ReturnInst (:number) 123: number
// CHECK-NEXT:function_end

// CHECK:function k(k: number): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:number) 123: number
// CHECK-NEXT:function_end

// CHECK:function "foo 2#"(cond: boolean, val: number): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %val: number
// CHECK-NEXT:  %1 = ReturnInst (:number) %0: number
// CHECK-NEXT:function_end

// CHECK:function bar(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:closure) [foo@load_store_multiple_test]: closure
// CHECK-NEXT:  %1 = CallInst (:number) %0: closure, empty: any, empty: any, undefined: undefined, true: boolean, 7: number
// CHECK-NEXT:  %2 = CallInst (:number) %0: closure, empty: any, empty: any, undefined: undefined, true: boolean, 8: number
// CHECK-NEXT:  %3 = BinaryAddInst (:number) %1: number, %2: number
// CHECK-NEXT:  %4 = ReturnInst (:number) %3: number
// CHECK-NEXT:function_end
