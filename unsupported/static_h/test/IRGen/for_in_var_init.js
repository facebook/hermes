/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermesc -dump-ir %s -O

function foo(obj) {
  for (var x = 10 in obj) {
      print(x);
  }
  return x;
}
//CHECK-LABEL: function foo(obj)
//CHECK-NEXT: frame = [x, obj]
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   %0 = StoreFrameInst undefined : undefined, [x]
//CHECK-NEXT:   %1 = StoreFrameInst %obj, [obj]
//CHECK-NEXT:   %2 = AllocStackInst $?anon_0_iter
//CHECK-NEXT:   %3 = AllocStackInst $?anon_1_base
//CHECK-NEXT:   %4 = AllocStackInst $?anon_2_idx
//CHECK-NEXT:   %5 = AllocStackInst $?anon_3_size
//CHECK-NEXT:   %6 = StoreFrameInst 10 : number, [x]
//CHECK-NEXT:   %7 = LoadFrameInst [obj]
//CHECK-NEXT:   %8 = StoreStackInst %7, %3
//CHECK-NEXT:   %9 = AllocStackInst $?anon_4_prop
//CHECK-NEXT:   %10 = GetPNamesInst %2, %3, %4, %5, %BB1, %BB2
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   %11 = LoadFrameInst [x]
//CHECK-NEXT:   %12 = ReturnInst %11
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   %13 = GetNextPNameInst %9, %3, %4, %5, %2, %BB1, %BB3
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   %14 = LoadStackInst %9
//CHECK-NEXT:   %15 = StoreFrameInst %14, [x]
//CHECK-NEXT:   %16 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHECK-NEXT:   %17 = LoadFrameInst [x]
//CHECK-NEXT:   %18 = CallInst %16, undefined : undefined, %17
//CHECK-NEXT:   %19 = BranchInst %BB2
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   %20 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
