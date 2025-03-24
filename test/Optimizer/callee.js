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
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "fuzz": string
// CHECK-NEXT:       DeclareGlobalVarInst "ctor_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_store_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_store_multiple_test": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) empty: any, empty: any, %fuzz(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %4: object, globalObject: object, "fuzz": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) empty: any, empty: any, %ctor_test(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %6: object, globalObject: object, "ctor_test": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) empty: any, empty: any, %load_store_test(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %8: object, globalObject: object, "load_store_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) empty: any, empty: any, %load_store_multiple_test(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %10: object, globalObject: object, "load_store_multiple_test": string
// CHECK-NEXT:        ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function fuzz(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:  %1 = CallInst (:number) %0: object, %foo(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number, 12: number
// CHECK-NEXT:       ReturnInst 12: number
// CHECK-NEXT:function_end

// CHECK:function ctor_test(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %"foo 1#"(): functionCode
// CHECK-NEXT:  %1 = CreateThisInst (:object) %0: object, %0: object
// CHECK-NEXT:  %2 = CallInst (:any) %0: object, %"foo 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, %1: object, 12: number
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:scope %VS0 [k: object]

// CHECK:function load_store_test(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %ping(): functionCode
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) empty: any, empty: any, %k(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %2: object, [%VS0.k]: object
// CHECK-NEXT:  %4 = CallInst (:number) %1: object, %ping(): functionCode, true: boolean, %0: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst 123: number
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [foo: object]

// CHECK:function load_store_multiple_test(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS1: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %"foo 2#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: object, [%VS1.foo]: object
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS1: any, %bar(): functionCode
// CHECK-NEXT:  %4 = CallInst (:number) %3: object, %bar(): functionCode, true: boolean, %0: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:function_end

// CHECK:function foo(k: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 12: number
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"(k: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %k: any
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end

// CHECK:function ping(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [%VS0.k]: object
// CHECK-NEXT:  %2 = CallInst (:number) %1: object, %k(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number, 123: number
// CHECK-NEXT:       ReturnInst 123: number
// CHECK-NEXT:function_end

// CHECK:function k(k: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 123: number
// CHECK-NEXT:function_end

// CHECK:function "foo 2#"(cond: boolean, val: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %val: number
// CHECK-NEXT:       ReturnInst %0: number
// CHECK-NEXT:function_end

// CHECK:function bar(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [%VS1.foo]: object
// CHECK-NEXT:  %2 = CallInst (:number) %1: object, %"foo 2#"(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number, true: boolean, 7: number
// CHECK-NEXT:  %3 = CallInst (:number) %1: object, %"foo 2#"(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number, true: boolean, 8: number
// CHECK-NEXT:  %4 = BinaryAddInst (:number) %2: number, %3: number
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:function_end
