/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xflow-parser -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s
// REQUIRES: flowparser

function foo() {
    let x = 20;
    const y = 30;
    return x + y;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:globals = [foo]
// CHECK-NEXT:S{global#0()#1} = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %foo#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "foo" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function foo#0#1()#2
// CHECK-NEXT:S{foo#0#1()#2} = [x#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{foo#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [x#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst undefined : undefined, [y#2], %0
// CHECK-NEXT:  %3 = StoreFrameInst 20 : number, [x#2], %0
// CHECK-NEXT:  %4 = StoreFrameInst 30 : number, [y#2], %0
// CHECK-NEXT:  %5 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %6 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %5, %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
