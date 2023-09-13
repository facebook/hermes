/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function foo() {
  return "hi"
  return 2.312
  return 12
  return 0x12
  return true
  return undefined
  return null
}

foo()

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): any
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreStackInst %6: any, %3: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %3: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst "hi": string
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst 2.312: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst 12: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       ReturnInst 18: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:       ReturnInst true: boolean
// CHECK-NEXT:%BB5:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB6:
// CHECK-NEXT:       ReturnInst null: null
// CHECK-NEXT:%BB7:
// CHECK-NEXT:       UnreachableInst
// CHECK-NEXT:function_end
