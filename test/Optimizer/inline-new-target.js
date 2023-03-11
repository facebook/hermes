/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function outer(a, b) {
    'use strict'
    function f1() {
        // new.target leaks the closure, so it may prevent inlining.
        return new.target;
    }
    return f1();
}

function outer2(){
    function foo(){
      'inline'
      return new.target;
    }
    function bar(){
      return foo();
    }
    return new bar();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "outer2": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %outer(): undefined|closure
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: closure, globalObject: object, "outer": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %outer2(): object
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "outer2": string
// CHECK-NEXT:  %6 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer(a: any, b: any): undefined|closure
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %f1(): undefined|closure
// CHECK-NEXT:  %1 = CallInst (:undefined|closure) %0: closure, %f1(): undefined|closure, empty: any, undefined: undefined
// CHECK-NEXT:  %2 = ReturnInst %1: undefined|closure
// CHECK-NEXT:function_end

// CHECK:function outer2(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %bar(): undefined
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: closure, "prototype": string
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: any, %0: closure
// CHECK-NEXT:  %3 = ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function f1(): undefined|closure
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:undefined|closure) %new.target: undefined|closure
// CHECK-NEXT:  %1 = ReturnInst %0: undefined|closure
// CHECK-NEXT:function_end

// CHECK:function bar(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
