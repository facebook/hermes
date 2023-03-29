/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Test that the variable `f` is initialized to undefined at the start of the
// scope.

function foo() {
  init = f;
  {
    function f() {}
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = [f: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [f]: any
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [f]: any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: any, globalObject: object, "init": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:closure) %f(): any
// CHECK-NEXT:  %4 = StoreFrameInst %3: closure, [f]: any
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
