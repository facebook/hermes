/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O  | %FileCheckOrRegen %s

// Make sure that we are promoting t to a constant value in bar()
function foo(p1) {
  var t = 123;

  function bar() {
    return t;
  }

  return bar;
}

foo()()

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(p1: any): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %bar(): functionCode
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function bar(): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 123: number
// CHECK-NEXT:function_end
