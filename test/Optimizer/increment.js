/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O  | %FileCheckOrRegen %s

function foo() {
  var k = 4;
  k++
  k++
  k++
  k++
  k++
  k++
  return k
}

foo()

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %foo() : number
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = LoadPropertyInst globalObject : object, "foo" : string
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function foo() : number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 10 : number
// CHECK-NEXT:function_end
