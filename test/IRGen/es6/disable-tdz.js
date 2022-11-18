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

// CHECK:function global()
// CHECK-NEXT:frame = [], globals = [check1]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst %check1()
// CHECK-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "check1" : string
// CHECK-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHECK-NEXT:  %4 = LoadStackInst %2
// CHECK-NEXT:  %5 = ReturnInst %4
// CHECK-NEXT:function_end

// CHECK:function check1()
// CHECK-NEXT:frame = [x, y]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty : empty, [x]
// CHECK-NEXT:  %1 = StoreFrameInst empty : empty, [y]
// CHECK-NEXT:  %2 = LoadFrameInst [x]
// CHECK-NEXT:  %3 = ThrowIfEmptyInst %2
// CHECK-NEXT:  %4 = LoadFrameInst [y]
// CHECK-NEXT:  %5 = ThrowIfEmptyInst %4
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %3, %5
// CHECK-NEXT:  %7 = ReturnInst %6
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = StoreFrameInst 10 : number, [x]
// CHECK-NEXT:  %9 = StoreFrameInst 1 : number, [y]
// CHECK-NEXT:  %10 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHKDIS:function global()
// CHKDIS-NEXT:frame = [], globals = [check1]
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = CreateFunctionInst %check1()
// CHKDIS-NEXT:  %1 = StorePropertyLooseInst %0 : closure, globalObject : object, "check1" : string
// CHKDIS-NEXT:  %2 = AllocStackInst $?anon_0_ret
// CHKDIS-NEXT:  %3 = StoreStackInst undefined : undefined, %2
// CHKDIS-NEXT:  %4 = LoadStackInst %2
// CHKDIS-NEXT:  %5 = ReturnInst %4
// CHKDIS-NEXT:function_end

// CHKDIS:function check1()
// CHKDIS-NEXT:frame = [x, y]
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = StoreFrameInst undefined : undefined, [x]
// CHKDIS-NEXT:  %1 = StoreFrameInst undefined : undefined, [y]
// CHKDIS-NEXT:  %2 = LoadFrameInst [x]
// CHKDIS-NEXT:  %3 = LoadFrameInst [y]
// CHKDIS-NEXT:  %4 = BinaryOperatorInst '+', %2, %3
// CHKDIS-NEXT:  %5 = ReturnInst %4
// CHKDIS-NEXT:%BB1:
// CHKDIS-NEXT:  %6 = StoreFrameInst 10 : number, [x]
// CHKDIS-NEXT:  %7 = StoreFrameInst 1 : number, [y]
// CHKDIS-NEXT:  %8 = ReturnInst undefined : undefined
// CHKDIS-NEXT:function_end
