/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Make sure that Mem2Reg doesn't promote loads across writing
// instructions with AllocStackInst operands.

function foo(x) {
  var [a,b] = x;
  print(a,b)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %foo(): undefined
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "foo": string
// CHECK-NEXT:  %3 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:  %3 = StoreStackInst %0: any, %2: any
// CHECK-NEXT:  %4 = IteratorBeginInst (:any) %2: any
// CHECK-NEXT:  %5 = StoreStackInst %4: any, %1: any
// CHECK-NEXT:  %6 = IteratorNextInst (:any) %1: any, %2: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %8 = BinaryStrictlyEqualInst (:boolean) %7: any, undefined: undefined
// CHECK-NEXT:  %9 = CondBranchInst %8: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = PhiInst (:any) undefined: undefined, %BB0, %6: any, %BB2
// CHECK-NEXT:  %12 = CondBranchInst %8: boolean, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = IteratorNextInst (:any) %1: any, %2: any
// CHECK-NEXT:  %14 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %15 = BinaryStrictlyEqualInst (:boolean) %14: any, undefined: undefined
// CHECK-NEXT:  %16 = CondBranchInst %15: boolean, %BB3, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = PhiInst (:any) undefined: undefined, %BB1, undefined: undefined, %BB4, %13: any, %BB5
// CHECK-NEXT:  %19 = PhiInst (:boolean) %8: boolean, %BB1, %15: boolean, %BB4, %15: boolean, %BB5
// CHECK-NEXT:  %20 = CondBranchInst %19: boolean, %BB6, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %21 = IteratorCloseInst (:any) %1: any, false: boolean
// CHECK-NEXT:  %22 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %24 = CallInst (:any) %23: any, empty: any, empty: any, undefined: undefined, %11: any, %18: any
// CHECK-NEXT:  %25 = ReturnInst (:undefined) undefined: undefined
// CHECK-NEXT:function_end
