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
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %foo(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined
// CHECK-NEXT:  %7 = StoreStackInst %6: any, %3: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %9 = ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:function foo(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst "hi": string
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = ReturnInst 2.312: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %2 = ReturnInst 12: number
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %3 = ReturnInst 18: number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %4 = ReturnInst true: boolean
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %5 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %6 = ReturnInst null: null
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %7 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
