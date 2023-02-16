/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

function test_one(x,y,z) {
  x + y
  x + 5
  false + y
  8 + false
  9 + "9"
  8 + false
  "hi" + "bye"
  var t = "hi" + z
  null + "hi"
  return t
}

function test_two(x,y,z) {
  function test00() {}
  var test01 = function() {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "test_one": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "test_two": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %test_one(): string
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2: closure, globalObject: object, "test_one": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %test_two(): undefined
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4: closure, globalObject: object, "test_two": string
// CHECK-NEXT:  %6 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test_one(x: any, y: any, z: any): string
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  %2 = LoadParamInst (:any) %z: any
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// CHECK-NEXT:  %4 = BinaryAddInst (:string|number) %0: any, 5: number
// CHECK-NEXT:  %5 = BinaryAddInst (:string|number) false: boolean, %1: any
// CHECK-NEXT:  %6 = BinaryAddInst (:string) "hi": string, %2: any
// CHECK-NEXT:  %7 = ReturnInst (:string) %6: string
// CHECK-NEXT:function_end

// CHECK:function test_two(x: any, y: any, z: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end
