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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = StorePropertyInst 0 : number, globalObject : object, "x" : string
// CHECK-NEXT:  %4 = StorePropertyInst 0 : number, globalObject : object, "x" : string
// CHECK-NEXT:  %5 = StoreStackInst 0 : number, %1
// CHECK-NEXT:  %6 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = StorePropertyInst 2 : number, globalObject : object, "x" : string
// CHECK-NEXT:  %8 = StoreStackInst 2 : number, %1
// CHECK-NEXT:  %9 = LoadStackInst %1
// CHECK-NEXT:  %10 = ReturnInst %9
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %11 = StorePropertyInst 1 : number, globalObject : object, "x" : string
// CHECK-NEXT:  %12 = StoreStackInst 1 : number, %1
// CHECK-NEXT:  %13 = BranchInst %BB1
// CHECK-NEXT:function_end
