/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict'

function outer(a, b) {
    // This can be inlined even though it uses new.target, since it is never
    // invoked as a constructor.
    function f1() {
        return new.target;
    }
    return f1();
}

function outer2(){
    // This will be inlined even though the closure escapes, since it is marked
    // 'inline'.
    function foo(){
      'inline'
      return new.target;
    }
    function bar(){
      return new foo();
    }
    return new bar();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:       DeclareGlobalVarInst "outer2": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %outer(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %3: object, globalObject: object, "outer": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %outer2(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %5: object, globalObject: object, "outer2": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function outer(a: any, b: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer2(): object
// CHECK-NEXT:frame = [foo: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %outer2(): any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [foo]: object
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %bar(): functionCode
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: object, "prototype": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %2: object, "prototype": string
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function foo(): undefined|object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %0: undefined|object
// CHECK-NEXT:function_end

// CHECK:function bar(): object [allCallsitesKnownInStrictMode,unreachable]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end
