/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines
// RUN: %shermes -dump-ir %s -Xcustom-opt=functionanalysis,typeinference

// Make sure all of these terminate in both pipelines we test here.

'use strict';

function test1() {
  function f(x) {
    return f(x);
  }
}

function test2() {
  function f1(p1) {
    if (x || y) {
    }
    return p1;
  }
  function f2() {
    f1();
  }
  function f3() {
    var v1;
    for (;;) {
      v1 = f1(v1);
    }
  }
  function f4() {
    f3();
  }
}

function test3() {
  function f1(p1) {
    return p1;
  }
  return function f3() {
    var v1 = f1(v1);
  };
}

function test4(f) {
  return foo(function(p) {
    throw p;
  });
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "test1": string
// CHECK-NEXT:       DeclareGlobalVarInst "test2": string
// CHECK-NEXT:       DeclareGlobalVarInst "test3": string
// CHECK-NEXT:       DeclareGlobalVarInst "test4": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %test1(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %5: object, globalObject: object, "test1": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %test2(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %7: object, globalObject: object, "test2": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %test3(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %9: object, globalObject: object, "test3": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %test4(): functionCode
// CHECK-NEXT:        StorePropertyStrictInst %11: object, globalObject: object, "test4": string
// CHECK-NEXT:        ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function test1(): undefined
// CHECK-NEXT:frame = [f: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %test1(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %f(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [f]: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test2(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test3(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %"f3 1#"(): functionCode
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function test4(f: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %3 = CallInst (:any) %1: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %2: object
// CHECK-NEXT:       ReturnInst %3: any
// CHECK-NEXT:function_end

// CHECK:function f(x: any): any [allCallsitesKnownInStrictMode,unreachable]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end

// CHECK:function "f3 1#"(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(p: any): any [noReturn]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       ThrowInst %0: any
// CHECK-NEXT:function_end
