/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function test_assignment_expr() {
  var y = 0;
  var x = y = 4;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "test_assignment_expr" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %test_assignment_expr()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "test_assignment_expr" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function test_assignment_expr()
// CHECK-NEXT:frame = [y, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [y]
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [y]
// CHECK-NEXT:  %3 = StoreFrameInst 4 : number, [y]
// CHECK-NEXT:  %4 = StoreFrameInst 4 : number, [x]
// CHECK-NEXT:  %5 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
