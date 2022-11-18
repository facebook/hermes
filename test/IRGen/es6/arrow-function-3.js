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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [foo]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %foo()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function foo(x)
// CHECK-NEXT:frame = [?anon_0_this, ?anon_1_new.target, x]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst %this, [?anon_0_this]
// CHECK-NEXT:  %1 = GetNewTargetInst
// CHECK-NEXT:  %2 = StoreFrameInst %1, [?anon_1_new.target]
// CHECK-NEXT:  %3 = BinaryOperatorInst '!==', %x, undefined : undefined
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %5 = CreateFunctionInst %x()
// CHECK-NEXT:  %6 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = PhiInst %x, %BB0, %5 : closure, %BB2
// CHECK-NEXT:  %8 = StoreFrameInst %7, [x]
// CHECK-NEXT:  %9 = LoadFrameInst [x]
// CHECK-NEXT:  %10 = CallInst %9, undefined : undefined
// CHECK-NEXT:  %11 = ReturnInst %10
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:arrow x()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadFrameInst [?anon_0_this@foo]
// CHECK-NEXT:  %1 = ReturnInst %0
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
