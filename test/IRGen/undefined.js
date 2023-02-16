/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function foo() {
  var undefined = 5;
  return undefined;
}

var undefined = 5;
foo();
undefined;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "undefined": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %5 = StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = StorePropertyLooseInst 5: number, globalObject: object, "undefined": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %9 = StoreStackInst %8: any, %4: any
// CHECK-NEXT:  %10 = StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %11 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %12 = ReturnInst (:any) %11: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = [undefined: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [undefined]: any
// CHECK-NEXT:  %1 = StoreFrameInst 5: number, [undefined]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [undefined]: any
// CHECK-NEXT:  %3 = ReturnInst (:any) %2: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = ReturnInst (:any) undefined: undefined
// CHECK-NEXT:function_end
