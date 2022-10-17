/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(x = () => this) {
    return x();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1(x)#2
// CHECK-NEXT:frame = [?anon_0_this#2, ?anon_1_new.target#2, x#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst %this, [?anon_0_this#2], %0
// CHECK-NEXT:  %2 = GetNewTargetInst
// CHECK-NEXT:  %3 = StoreFrameInst %2, [?anon_1_new.target#2], %0
// CHECK-NEXT:  %4 = BinaryOperatorInst '!==', %x, undefined : undefined
// CHECK-NEXT:  %5 = CondBranchInst %4, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = CreateFunctionInst %x#1#2()#3, %0
// CHECK-NEXT:  %7 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = PhiInst %x, %BB0, %6 : closure, %BB2
// CHECK-NEXT:  %9 = StoreFrameInst %8, [x#2], %0
// CHECK-NEXT:  %10 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %11 = CallInst %10, undefined : undefined
// CHECK-NEXT:  %12 = ReturnInst %11
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow x#1#2()#3
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{x#1#2()#3}
// CHECK-NEXT:  %1 = LoadFrameInst [?anon_0_this#2@foo], %0
// CHECK-NEXT:  %2 = ReturnInst %1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
