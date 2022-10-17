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

// CHECK:function global#0()#1 : undefined
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2 : undefined, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(x)#2 : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_iter
// CHECK-NEXT:  %2 = AllocStackInst $?anon_1_sourceOrNext
// CHECK-NEXT:  %3 = StoreStackInst %x, %2
// CHECK-NEXT:  %4 = IteratorBeginInst %2
// CHECK-NEXT:  %5 = StoreStackInst %4, %1
// CHECK-NEXT:  %6 = IteratorNextInst %1, %2
// CHECK-NEXT:  %7 = LoadStackInst %1
// CHECK-NEXT:  %8 = BinaryOperatorInst '===', %7, undefined : undefined
// CHECK-NEXT:  %9 = CondBranchInst %8 : boolean, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %11 = PhiInst undefined : undefined, %BB0, %6, %BB2
// CHECK-NEXT:  %12 = CondBranchInst %8 : boolean, %BB3, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = IteratorNextInst %1, %2
// CHECK-NEXT:  %14 = LoadStackInst %1
// CHECK-NEXT:  %15 = BinaryOperatorInst '===', %14, undefined : undefined
// CHECK-NEXT:  %16 = CondBranchInst %15 : boolean, %BB3, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = PhiInst undefined : undefined, %BB1, undefined : undefined, %BB4, %13, %BB5
// CHECK-NEXT:  %19 = PhiInst %8 : boolean, %BB1, %15 : boolean, %BB4, %15 : boolean, %BB5
// CHECK-NEXT:  %20 = CondBranchInst %19 : boolean, %BB6, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %21 = IteratorCloseInst %1, false : boolean
// CHECK-NEXT:  %22 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %24 = CallInst %23, undefined : undefined, %11, %18
// CHECK-NEXT:  %25 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
