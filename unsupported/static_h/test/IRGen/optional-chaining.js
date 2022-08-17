/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck %s --match-full-lines

function f1(a) {
  return a?.b;
}
// CHECK-LABEL: function f1(a)
// CHECK-NEXT: frame = [a]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = LoadFrameInst [a]
// CHECK-NEXT:   %2 = BinaryOperatorInst '==', %1, null : null
// CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %4 = PhiInst undefined : undefined, %BB1, %7, %BB2
// CHECK-NEXT:   %5 = ReturnInst %4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %6 = BranchInst %BB3
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %7 = LoadPropertyInst %1, "b" : string
// CHECK-NEXT:   %8 = BranchInst %BB3
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %9 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function f2(f) {
  return f?.();
}
// CHECK-LABEL: function f2(f)
// CHECK-NEXT: frame = [f]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %f, [f]
// CHECK-NEXT:   %1 = LoadFrameInst [f]
// CHECK-NEXT:   %2 = BinaryOperatorInst '==', %1, null : null
// CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %4 = PhiInst undefined : undefined, %BB1, %7, %BB2
// CHECK-NEXT:   %5 = ReturnInst %4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %6 = BranchInst %BB3
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %7 = CallInst %1, undefined : undefined
// CHECK-NEXT:   %8 = BranchInst %BB3
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %9 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function f3(a) {
  return a?.b.c;
}
// CHECK-LABEL: function f3(a)
// CHECK-NEXT: frame = [a]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = LoadFrameInst [a]
// CHECK-NEXT:   %2 = BinaryOperatorInst '==', %1, null : null
// CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %4 = PhiInst undefined : undefined, %BB1, %8, %BB2
// CHECK-NEXT:   %5 = ReturnInst %4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %6 = BranchInst %BB3
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %7 = LoadPropertyInst %1, "b" : string
// CHECK-NEXT:   %8 = LoadPropertyInst %7, "c" : string
// CHECK-NEXT:   %9 = BranchInst %BB3
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %10 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function f4(a) {
  return a?.b().c;
}
// CHECK-LABEL: function f4(a)
// CHECK-NEXT: frame = [a]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = LoadFrameInst [a]
// CHECK-NEXT:   %2 = BinaryOperatorInst '==', %1, null : null
// CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %4 = PhiInst undefined : undefined, %BB1, %9, %BB2
// CHECK-NEXT:   %5 = ReturnInst %4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %6 = BranchInst %BB3
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %7 = LoadPropertyInst %1, "b" : string
// CHECK-NEXT:   %8 = CallInst %7, %1
// CHECK-NEXT:   %9 = LoadPropertyInst %8, "c" : string
// CHECK-NEXT:   %10 = BranchInst %BB3
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %11 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function f5(a) {
  return a?.().b;
}
// CHECK-LABEL: function f5(a)
// CHECK-NEXT: frame = [a]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = LoadFrameInst [a]
// CHECK-NEXT:   %2 = BinaryOperatorInst '==', %1, null : null
// CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %4 = PhiInst undefined : undefined, %BB1, %8, %BB2
// CHECK-NEXT:   %5 = ReturnInst %4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %6 = BranchInst %BB3
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %7 = CallInst %1, undefined : undefined
// CHECK-NEXT:   %8 = LoadPropertyInst %7, "b" : string
// CHECK-NEXT:   %9 = BranchInst %BB3
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %10 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function f6(a) {
  return (a?.b.c).d;
}
// CHECK-LABEL: function f6(a)
// CHECK-NEXT: frame = [a]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = LoadFrameInst [a]
// CHECK-NEXT:   %2 = BinaryOperatorInst '==', %1, null : null
// CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %4 = PhiInst undefined : undefined, %BB1, %9, %BB2
// CHECK-NEXT:   %5 = LoadPropertyInst %4, "d" : string
// CHECK-NEXT:   %6 = ReturnInst %5
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %7 = BranchInst %BB3
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %8 = LoadPropertyInst %1, "b" : string
// CHECK-NEXT:   %9 = LoadPropertyInst %8, "c" : string
// CHECK-NEXT:   %10 = BranchInst %BB3
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %11 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function f7(a) {
  return a?.b?.().c;
}
// CHECK-LABEL: function f7(a)
// CHECK-NEXT: frame = [a]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = LoadFrameInst [a]
// CHECK-NEXT:   %2 = BinaryOperatorInst '==', %1, null : null
// CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %4 = PhiInst undefined : undefined, %BB1, %11, %BB4
// CHECK-NEXT:   %5 = ReturnInst %4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %6 = BranchInst %BB3
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %7 = LoadPropertyInst %1, "b" : string
// CHECK-NEXT:   %8 = BinaryOperatorInst '==', %7, null : null
// CHECK-NEXT:   %9 = CondBranchInst %8, %BB1, %BB4
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %10 = CallInst %7, %1
// CHECK-NEXT:   %11 = LoadPropertyInst %10, "c" : string
// CHECK-NEXT:   %12 = BranchInst %BB3
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %13 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function f8(a) {
  return a?.b?.c?.();
}
// CHECK-LABEL: function f8(a)
// CHECK-NEXT: frame = [a]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = LoadFrameInst [a]
// CHECK-NEXT:   %2 = BinaryOperatorInst '==', %1, null : null
// CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %4 = PhiInst undefined : undefined, %BB1, %13, %BB4
// CHECK-NEXT:   %5 = ReturnInst %4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %6 = BranchInst %BB3
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %7 = LoadPropertyInst %1, "b" : string
// CHECK-NEXT:   %8 = BinaryOperatorInst '==', %7, null : null
// CHECK-NEXT:   %9 = CondBranchInst %8, %BB1, %BB5
// CHECK-NEXT: %BB5:
// CHECK-NEXT:   %10 = LoadPropertyInst %7, "c" : string
// CHECK-NEXT:   %11 = BinaryOperatorInst '==', %10, null : null
// CHECK-NEXT:   %12 = CondBranchInst %11, %BB1, %BB4
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %13 = CallInst %10, %7
// CHECK-NEXT:   %14 = BranchInst %BB3
// CHECK-NEXT: %BB6:
// CHECK-NEXT:   %15 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

function f9(a) {
  return delete a?.b;
}
// CHECK-LABEL: function f9(a)
// CHECK-NEXT: frame = [a]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %a, [a]
// CHECK-NEXT:   %1 = LoadFrameInst [a]
// CHECK-NEXT:   %2 = BinaryOperatorInst '==', %1, null : null
// CHECK-NEXT:   %3 = CondBranchInst %2, %BB1, %BB2
// CHECK-NEXT: %BB3:
// CHECK-NEXT:   %4 = PhiInst undefined : undefined, %BB1, %7, %BB2
// CHECK-NEXT:   %5 = ReturnInst %4
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %6 = BranchInst %BB3
// CHECK-NEXT: %BB2:
// CHECK-NEXT:   %7 = DeletePropertyInst %1, "b" : string
// CHECK-NEXT:   %8 = BranchInst %BB3
// CHECK-NEXT: %BB4:
// CHECK-NEXT:   %9 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end
