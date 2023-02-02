/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

function f1(a, b) {
  return a ?? b;
}

function f2(a, b) {
  if (a ?? b) {
    return 1;
  } else {
    return 2;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "f1" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "f2" : string
// CHECK-NEXT:  %2 = CreateFunctionInst %f1()
// CHECK-NEXT:  %3 = StorePropertyLooseInst %2 : closure, globalObject : object, "f1" : string
// CHECK-NEXT:  %4 = CreateFunctionInst %f2()
// CHECK-NEXT:  %5 = StorePropertyLooseInst %4 : closure, globalObject : object, "f2" : string
// CHECK-NEXT:  %6 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %7 = StoreStackInst undefined : undefined, %6
// CHECK-NEXT:  %8 = LoadStackInst %6
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end

// CHECK:function f1(a, b)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadParamInst %b
// CHECK-NEXT:  %3 = StoreFrameInst %2, [b]
// CHECK-NEXT:  %4 = AllocStackInst $?anon_0_logical
// CHECK-NEXT:  %5 = LoadFrameInst [a]
// CHECK-NEXT:  %6 = StoreStackInst %5, %4
// CHECK-NEXT:  %7 = BinaryOperatorInst '==', %5, null : null
// CHECK-NEXT:  %8 = CondBranchInst %7, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst [b]
// CHECK-NEXT:  %10 = StoreStackInst %9, %4
// CHECK-NEXT:  %11 = BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = LoadStackInst %4
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function f2(a, b)
// CHECK-NEXT:frame = [a, b]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst %a
// CHECK-NEXT:  %1 = StoreFrameInst %0, [a]
// CHECK-NEXT:  %2 = LoadParamInst %b
// CHECK-NEXT:  %3 = StoreFrameInst %2, [b]
// CHECK-NEXT:  %4 = LoadFrameInst [a]
// CHECK-NEXT:  %5 = BinaryOperatorInst '==', %4, null : null
// CHECK-NEXT:  %6 = CondBranchInst %5, %BB1, %BB2
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %7 = ReturnInst 1 : number
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %8 = ReturnInst 2 : number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = LoadFrameInst [b]
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = CondBranchInst %4, %BB3, %BB4
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = BranchInst %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %14 = BranchInst %BB5
// CHECK-NEXT:function_end
