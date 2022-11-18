/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

var x = 0;
end: {
  x = 0;
  break end;
  x = 1;
}
x = 2;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %1 = StoreStackInst undefined : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyLooseInst 0 : number, globalObject : object, "x" : string
// CHECK-NEXT:  %3 = StorePropertyLooseInst 0 : number, globalObject : object, "x" : string
// CHECK-NEXT:  %4 = StoreStackInst 0 : number, %0
// CHECK-NEXT:  %5 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %6 = StorePropertyLooseInst 2 : number, globalObject : object, "x" : string
// CHECK-NEXT:  %7 = StoreStackInst 2 : number, %0
// CHECK-NEXT:  %8 = LoadStackInst %0
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = StorePropertyLooseInst 1 : number, globalObject : object, "x" : string
// CHECK-NEXT:  %11 = StoreStackInst 1 : number, %0
// CHECK-NEXT:  %12 = BranchInst %BB1
// CHECK-NEXT:function_end
