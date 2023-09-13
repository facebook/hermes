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
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:       DeclareGlobalVarInst "outer2": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %outer(): undefined
// CHECK-NEXT:       StorePropertyStrictInst %2: object, globalObject: object, "outer": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %outer2(): object
// CHECK-NEXT:       StorePropertyStrictInst %4: object, globalObject: object, "outer2": string
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
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %foo(): undefined|object
// CHECK-NEXT:       StoreFrameInst %0: object, [foo]: object
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %bar(): object
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "prototype": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function foo(): undefined|object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       ReturnInst %0: undefined|object
// CHECK-NEXT:function_end

// CHECK:function bar(): object [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst (:object) [foo@outer2]: object
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end
