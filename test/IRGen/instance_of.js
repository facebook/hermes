/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -dump-ir %s -O

function simple_test0(x, y) {
  return x instanceof y;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [simple_test0]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %simple_test0()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "simple_test0" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function simple_test0(x, y)
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
