/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -hermes-parser -dump-ir %s -dump-instr-uselist  | %FileCheckOrRegen %s --match-full-lines

function foo(a, b) {
  var c = a + b;
  return c * c;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any // users: %2
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any // users: %4 %5
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any // users: %6
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any
// CHECK-NEXT:frame = [a: any, b: any, c: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any // users: %1
// CHECK-NEXT:       StoreFrameInst %0: any, [a]: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %b: any // users: %3
// CHECK-NEXT:       StoreFrameInst %2: any, [b]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [c]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [a]: any // users: %7
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [b]: any // users: %7
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %5: any, %6: any // users: %8
// CHECK-NEXT:       StoreFrameInst %7: any, [c]: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [c]: any // users: %11
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [c]: any // users: %11
// CHECK-NEXT:  %11 = BinaryMultiplyInst (:any) %9: any, %10: any // users: %12
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end
