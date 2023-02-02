/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -dump-ir %s -O

function foo(obj) {
  for (var x = 10 in obj) {
      print(x);
  }
  return x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %foo()
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo(obj)
// CHECK-NEXT:frame = [obj, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %obj
// CHECK-NEXT:  %1 = StoreFrameInst %0, [obj]
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [x]
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %4 = AllocStackInst $?anon_1_base
// CHECK-NEXT:  %5 = AllocStackInst $?anon_2_idx
// CHECK-NEXT:  %6 = AllocStackInst $?anon_3_size
// CHECK-NEXT:  %7 = StoreFrameInst 10 : number, [x]
// CHECK-NEXT:  %8 = LoadFrameInst [obj]
// CHECK-NEXT:  %9 = StoreStackInst %8, %4
// CHECK-NEXT:  %10 = AllocStackInst $?anon_4_prop
// CHECK-NEXT:  %11 = GetPNamesInst %3, %4, %5, %6, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = LoadFrameInst [x]
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = GetNextPNameInst %10, %4, %5, %6, %3, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadStackInst %10
// CHECK-NEXT:  %16 = StoreFrameInst %15, [x]
// CHECK-NEXT:  %17 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %18 = LoadFrameInst [x]
// CHECK-NEXT:  %19 = CallInst %17, undefined : undefined, %18
// CHECK-NEXT:  %20 = BranchInst %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %21 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
