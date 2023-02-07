/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function fibonacci(n) {
   if (n){
     return n;
   } else {
     return fibonacci(n);
   }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "fibonacci" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %fibonacci()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "fibonacci" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function fibonacci(n)
// CHECK-NEXT:frame = [n]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %n
// CHECK-NEXT:  %1 = StoreFrameInst %0, [n]
// CHECK-NEXT:  %2 = LoadFrameInst [n]
// CHECK-NEXT:  %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = LoadFrameInst [n]
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadPropertyInst globalObject : object, "fibonacci" : string
// CHECK-NEXT:  %7 = LoadFrameInst [n]
// CHECK-NEXT:  %8 = CallInst %6, undefined : undefined, %7
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:function_end
