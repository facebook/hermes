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
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %fuzz(): number
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4: object, globalObject: object, "fuzz": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %ctor_test(): object
// CHECK-NEXT:  %7 = StorePropertyStrictInst %6: object, globalObject: object, "ctor_test": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %load_store_test(): number
// CHECK-NEXT:  %9 = StorePropertyStrictInst %8: object, globalObject: object, "load_store_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %load_store_multiple_test(): number
// CHECK-NEXT:  %11 = StorePropertyStrictInst %10: object, globalObject: object, "load_store_multiple_test": string
// CHECK-NEXT:  %12 = ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function fuzz(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %foo(): number
// CHECK-NEXT:  %1 = CallInst (:number) %0: object, %foo(): number, empty: any, undefined: undefined, 0: number, 12: number
// CHECK-NEXT:  %2 = ReturnInst 12: number
// CHECK-NEXT:function_end

// CHECK:function ctor_test(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %"foo 1#"(): number
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: any, %0: object
// CHECK-NEXT:  %3 = CallInst (:number) %0: object, %"foo 1#"(): number, empty: any, %0: object, 0: number, 12: number
// CHECK-NEXT:  %4 = ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function load_store_test(): number
// CHECK-NEXT:frame = [k: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %ping(): number
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %k(): number
// CHECK-NEXT:  %2 = StoreFrameInst %1: object, [k]: object
// CHECK-NEXT:  %3 = CallInst (:number) %0: object, %ping(): number, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = ReturnInst 123: number
// CHECK-NEXT:function_end

// CHECK:function load_store_multiple_test(): number
// CHECK-NEXT:frame = [foo: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %"foo 2#"(): number
// CHECK-NEXT:  %1 = StoreFrameInst %0: object, [foo]: object
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %bar(): number
// CHECK-NEXT:  %3 = CallInst (:number) %2: object, %bar(): number, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:  %4 = ReturnInst %3: number
// CHECK-NEXT:function_end

// CHECK:function foo(k: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 12: number
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"(k: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 12: number
// CHECK-NEXT:function_end

// CHECK:function ping(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:object) [k@load_store_test]: object
// CHECK-NEXT:  %1 = CallInst (:number) %0: object, %k(): number, empty: any, undefined: undefined, 0: number, 123: number
// CHECK-NEXT:  %2 = ReturnInst 123: number
// CHECK-NEXT:function_end

// CHECK:function k(k: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 123: number
// CHECK-NEXT:function_end

// CHECK:function "foo 2#"(cond: boolean, val: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %val: number
// CHECK-NEXT:  %1 = ReturnInst %0: number
// CHECK-NEXT:function_end

// CHECK:function bar(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:object) [foo@load_store_multiple_test]: object
// CHECK-NEXT:  %1 = CallInst (:number) %0: object, %"foo 2#"(): number, empty: any, undefined: undefined, 0: number, true: boolean, 7: number
// CHECK-NEXT:  %2 = CallInst (:number) %0: object, %"foo 2#"(): number, empty: any, undefined: undefined, 0: number, true: boolean, 8: number
// CHECK-NEXT:  %3 = BinaryAddInst (:number) %1: number, %2: number
// CHECK-NEXT:  %4 = ReturnInst %3: number
// CHECK-NEXT:function_end
