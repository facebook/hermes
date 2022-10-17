/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -Xenable-tdz -dump-ir %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines --check-prefix=CHKDIS %s

function check1() {
    return x + y;
    let x = 10;
    const y = 1;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [check1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %check1#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "check1" : string
// CHECK-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHECK-NEXT:  %5 = LoadStackInst %3
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end

// CHECK:function check1#0#1()#2
// CHECK-NEXT:frame = [x#2, y#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{check1#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst empty : empty, [x#2], %0
// CHECK-NEXT:  %2 = StoreFrameInst empty : empty, [y#2], %0
// CHECK-NEXT:  %3 = LoadFrameInst [x#2], %0
// CHECK-NEXT:  %4 = ThrowIfEmptyInst %3
// CHECK-NEXT:  %5 = LoadFrameInst [y#2], %0
// CHECK-NEXT:  %6 = ThrowIfEmptyInst %5
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %4, %6
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = StoreFrameInst 10 : number, [x#2], %0
// CHECK-NEXT:  %10 = StoreFrameInst 1 : number, [y#2], %0
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHKDIS:function global#0()#1
// CHKDIS-NEXT:frame = [], globals = [check1]
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHKDIS-NEXT:  %1 = CreateFunctionInst %check1#0#1()#2, %0
// CHKDIS-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "check1" : string
// CHKDIS-NEXT:  %3 = AllocStackInst $?anon_0_ret
// CHKDIS-NEXT:  %4 = StoreStackInst undefined : undefined, %3
// CHKDIS-NEXT:  %5 = LoadStackInst %3
// CHKDIS-NEXT:  %6 = ReturnInst %5
// CHKDIS-NEXT:function_end

// CHKDIS:function check1#0#1()#2
// CHKDIS-NEXT:frame = [x#2, y#2]
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = CreateScopeInst %S{check1#0#1()#2}
// CHKDIS-NEXT:  %1 = StoreFrameInst undefined : undefined, [x#2], %0
// CHKDIS-NEXT:  %2 = StoreFrameInst undefined : undefined, [y#2], %0
// CHKDIS-NEXT:  %3 = LoadFrameInst [x#2], %0
// CHKDIS-NEXT:  %4 = LoadFrameInst [y#2], %0
// CHKDIS-NEXT:  %5 = BinaryOperatorInst '+', %3, %4
// CHKDIS-NEXT:  %6 = ReturnInst %5
// CHKDIS-NEXT:%BB1:
// CHKDIS-NEXT:  %7 = StoreFrameInst 10 : number, [x#2], %0
// CHKDIS-NEXT:  %8 = StoreFrameInst 1 : number, [y#2], %0
// CHKDIS-NEXT:  %9 = ReturnInst undefined : undefined
// CHKDIS-NEXT:function_end
