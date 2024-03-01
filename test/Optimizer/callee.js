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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "fuzz": string
// CHECK-NEXT:       DeclareGlobalVarInst "ctor_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_store_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "load_store_multiple_test": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %fuzz(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %5: object, globalObject: object, "fuzz": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %ctor_test(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %7: object, globalObject: object, "ctor_test": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %load_store_test(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %9: object, globalObject: object, "load_store_test": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %load_store_multiple_test(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %11: object, globalObject: object, "load_store_multiple_test": string
// CHECK-NEXT:        ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function fuzz(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:  %2 = CallInst (:number) %1: object, %foo(): functionCode, %0: environment, undefined: undefined, 0: number, 12: number
// CHECK-NEXT:       ReturnInst 12: number
// CHECK-NEXT:function_end

// CHECK:function ctor_test(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %"foo 1#"(): functionCode
// CHECK-NEXT:  %2 = LoadPropertyInst (:any) %1: object, "prototype": string
// CHECK-NEXT:  %3 = CreateThisInst (:object) %2: any, %1: object
// CHECK-NEXT:  %4 = CallInst (:number) %1: object, %"foo 1#"(): functionCode, %0: environment, undefined: undefined, 0: number, 12: number
// CHECK-NEXT:       ReturnInst %3: object
// CHECK-NEXT:function_end

// CHECK:function load_store_test(): number
// CHECK-NEXT:frame = [k: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %load_store_test(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %ping(): functionCode
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %1: environment, %k(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [k]: object
// CHECK-NEXT:  %5 = CallInst (:number) %2: object, %ping(): functionCode, %1: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst 123: number
// CHECK-NEXT:function_end

// CHECK:function load_store_multiple_test(): number
// CHECK-NEXT:frame = [foo: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %load_store_multiple_test(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %"foo 2#"(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [foo]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %bar(): functionCode
// CHECK-NEXT:  %5 = CallInst (:number) %4: object, %bar(): functionCode, %1: environment, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:function foo(k: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 12: number
// CHECK-NEXT:function_end

// CHECK:function "foo 1#"(k: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 12: number
// CHECK-NEXT:function_end

// CHECK:function ping(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %load_store_test(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [k@load_store_test]: object
// CHECK-NEXT:  %2 = CallInst (:number) %1: object, %k(): functionCode, empty: any, undefined: undefined, 0: number, 123: number
// CHECK-NEXT:       ReturnInst 123: number
// CHECK-NEXT:function_end

// CHECK:function k(k: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 123: number
// CHECK-NEXT:function_end

// CHECK:function "foo 2#"(cond: boolean, val: number): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %val: number
// CHECK-NEXT:       ReturnInst %0: number
// CHECK-NEXT:function_end

// CHECK:function bar(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %load_store_multiple_test(): any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [foo@load_store_multiple_test]: object
// CHECK-NEXT:  %2 = CallInst (:number) %1: object, %"foo 2#"(): functionCode, empty: any, undefined: undefined, 0: number, true: boolean, 7: number
// CHECK-NEXT:  %3 = CallInst (:number) %1: object, %"foo 2#"(): functionCode, empty: any, undefined: undefined, 0: number, true: boolean, 8: number
// CHECK-NEXT:  %4 = BinaryAddInst (:number) %2: number, %3: number
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:function_end
