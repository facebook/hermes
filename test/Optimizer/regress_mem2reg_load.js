/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

// Make sure that Mem2Reg doesn't promote loads across writing
// instructions with AllocStackInst operands.

function foo(x) {
  var [a,b] = x;
  print(a,b)
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global() : undefined
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo() : undefined
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo(x) : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %1 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %2 = StoreStackInst %x, %1
// CHECK-NEXT:  %3 = IteratorBeginInst %1
// CHECK-NEXT:  %4 = StoreStackInst %3, %0
// CHECK-NEXT:  %5 = IteratorNextInst %0, %1
// CHECK-NEXT:  %6 = LoadStackInst %0
// CHECK-NEXT:  %7 = BinaryOperatorInst '===', %6, undefined : undefined
// CHECK-NEXT:  %8 = CondBranchInst %7 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %9 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %10 = PhiInst undefined : undefined, %BB0, %5, %BB2
// CHECK-NEXT:  %11 = CondBranchInst %7 : boolean, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = IteratorNextInst %0, %1
// CHECK-NEXT:  %13 = LoadStackInst %0
// CHECK-NEXT:  %14 = BinaryOperatorInst '===', %13, undefined : undefined
// CHECK-NEXT:  %15 = CondBranchInst %14 : boolean, %BB3, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %16 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = PhiInst undefined : undefined, %BB1, undefined : undefined, %BB4, %12, %BB5
// CHECK-NEXT:  %18 = PhiInst %7 : boolean, %BB1, %14 : boolean, %BB4, %14 : boolean, %BB5
// CHECK-NEXT:  %19 = CondBranchInst %18 : boolean, %BB6, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %20 = IteratorCloseInst %0, false : boolean
// CHECK-NEXT:  %21 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %23 = CallInst %22, undefined : undefined, %10, %17
// CHECK-NEXT:  %24 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
