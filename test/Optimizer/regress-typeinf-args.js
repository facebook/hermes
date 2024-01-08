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
// CHECK-NEXT:       DeclareGlobalVarInst "test1": string
// CHECK-NEXT:       DeclareGlobalVarInst "test2": string
// CHECK-NEXT:       DeclareGlobalVarInst "test3": string
// CHECK-NEXT:       DeclareGlobalVarInst "test4": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %test1(): undefined
// CHECK-NEXT:       StorePropertyStrictInst %4: object, globalObject: object, "test1": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %test2(): undefined
// CHECK-NEXT:       StorePropertyStrictInst %6: object, globalObject: object, "test2": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %test3(): object
// CHECK-NEXT:       StorePropertyStrictInst %8: object, globalObject: object, "test3": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %test4(): any
// CHECK-NEXT:        StorePropertyStrictInst %10: object, globalObject: object, "test4": string
// CHECK-NEXT:        ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function test1(): undefined
// CHECK-NEXT:frame = [f: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %f(): any
// CHECK-NEXT:       StoreFrameInst %0: object, [f]: object
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
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %"f3 1#"(): undefined
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function test4(f: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: object
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function f(x: any): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadFrameInst (:object) [f@test1]: object
// CHECK-NEXT:  %2 = CallInst (:any) %1: object, %f(): any, empty: any, undefined: undefined, 0: number, %0: any
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:function "f3 1#"(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(p: any): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %p: any
// CHECK-NEXT:       ThrowInst %0: any
// CHECK-NEXT:function_end
