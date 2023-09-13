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
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %foo(): undefined
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(x: any): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_1_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %0: any, %2: any
// CHECK-NEXT:  %4 = IteratorBeginInst (:any) %2: any
// CHECK-NEXT:       StoreStackInst %4: any, %1: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %2: any
// CHECK-NEXT:  %7 = IteratorNextInst (:any) %1: any, %6: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %9 = BinaryStrictlyEqualInst (:boolean) %8: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %12 = PhiInst (:any) undefined: undefined, %BB0, %7: any, %BB2
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = LoadStackInst (:any) %2: any
// CHECK-NEXT:  %15 = IteratorNextInst (:any) %1: any, %14: any
// CHECK-NEXT:  %16 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %17 = BinaryStrictlyEqualInst (:boolean) %16: any, undefined: undefined
// CHECK-NEXT:        CondBranchInst %17: boolean, %BB3, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = PhiInst (:any) undefined: undefined, %BB1, undefined: undefined, %BB4, %15: any, %BB5
// CHECK-NEXT:  %21 = PhiInst (:boolean) %9: boolean, %BB1, %17: boolean, %BB4, %17: boolean, %BB5
// CHECK-NEXT:        CondBranchInst %21: boolean, %BB6, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %23 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %24 = IteratorCloseInst (:any) %23: any, false: boolean
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %27 = CallInst (:any) %26: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %12: any, %20: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
