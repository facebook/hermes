/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


function fibonacci(n) {
   if (n){
     return n;
   } else {
     return fibonacci(n);
   }
}

//CHECK: function global()
//CHECK: frame = [], globals = [fibonacci]
//CHECK:   %BB0:
//CHECK:     %0 = CreateFunctionInst %fibonacci()
//CHECK:     %1 = StorePropertyInst %0 : closure, globalObject : object, "fibonacci" : string
//CHECK:     %2 = AllocStackInst $?anon_0_ret
//CHECK:     %3 = StoreStackInst undefined : undefined, %2
//CHECK:     %4 = LoadStackInst %2
//CHECK:     %5 = ReturnInst %4
//CHECK: function fibonacci(n)
//CHECK: frame = [n]
//CHECK:   %BB0:
//CHECK:     %0 = StoreFrameInst %n, [n]
//CHECK:     %1 = LoadFrameInst [n]
//CHECK:     %2 = CondBranchInst %1, %BB1, %BB2
//CHECK:   %BB1:
//CHECK:     %3 = LoadFrameInst [n]
//CHECK:     %4 = ReturnInst %3
//CHECK:   %BB2:
//CHECK:     %5 = LoadPropertyInst globalObject : object, "fibonacci" : string
//CHECK:     %6 = LoadFrameInst [n]
//CHECK:     %7 = CallInst %5, undefined : undefined, %6
//CHECK:     %8 = ReturnInst %7
//CHECK:   %BB3:
//CHECK:     %9 = ReturnInst undefined : undefined
//CHECK:   %BB4:
//CHECK:     %10 = BranchInst %BB3
//CHECK:   %BB5:
//CHECK:     %11 = BranchInst %BB3

