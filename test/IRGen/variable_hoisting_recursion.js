/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function fibonacci(n) {
   if (n){
     return n;
   } else {
     return fibonacci(n);
   }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [fibonacci]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %fibonacci()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "fibonacci" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function fibonacci(n)
// CHECK-NEXT:frame = [n]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %n, [n]
// CHECK-NEXT:  %1 = LoadFrameInst [n]
// CHECK-NEXT:  %2 = CondBranchInst %1, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = LoadFrameInst [n]
// CHECK-NEXT:  %4 = ReturnInst %3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = LoadPropertyInst globalObject : object, "fibonacci" : string
// CHECK-NEXT:  %6 = LoadFrameInst [n]
// CHECK-NEXT:  %7 = CallInst %5, undefined : undefined, %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:function_end
