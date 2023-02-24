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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "check1": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %check1(): any
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "check1": string
// CHECK-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHECK-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHECK-NEXT:  %6 = ReturnInst %5: any
// CHECK-NEXT:function_end

// CHECK:function check1(): any
// CHECK-NEXT:frame = [x: any, y: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst empty: empty, [x]: any
// CHECK-NEXT:  %1 = StoreFrameInst empty: empty, [y]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %3 = ThrowIfEmptyInst (:any) %2: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [y]: any
// CHECK-NEXT:  %5 = ThrowIfEmptyInst (:any) %4: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %3: any, %5: any
// CHECK-NEXT:  %7 = ReturnInst %6: any
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %8 = StoreFrameInst 10: number, [x]: any
// CHECK-NEXT:  %9 = StoreFrameInst 1: number, [y]: any
// CHECK-NEXT:  %10 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHKDIS:function global(): any
// CHKDIS-NEXT:frame = []
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = DeclareGlobalVarInst "check1": string
// CHKDIS-NEXT:  %1 = CreateFunctionInst (:closure) %check1(): any
// CHKDIS-NEXT:  %2 = StorePropertyLooseInst %1: closure, globalObject: object, "check1": string
// CHKDIS-NEXT:  %3 = AllocStackInst (:any) $?anon_0_ret: any
// CHKDIS-NEXT:  %4 = StoreStackInst undefined: undefined, %3: any
// CHKDIS-NEXT:  %5 = LoadStackInst (:any) %3: any
// CHKDIS-NEXT:  %6 = ReturnInst %5: any
// CHKDIS-NEXT:function_end

// CHKDIS:function check1(): any
// CHKDIS-NEXT:frame = [x: any, y: any]
// CHKDIS-NEXT:%BB0:
// CHKDIS-NEXT:  %0 = StoreFrameInst undefined: undefined, [x]: any
// CHKDIS-NEXT:  %1 = StoreFrameInst undefined: undefined, [y]: any
// CHKDIS-NEXT:  %2 = LoadFrameInst (:any) [x]: any
// CHKDIS-NEXT:  %3 = LoadFrameInst (:any) [y]: any
// CHKDIS-NEXT:  %4 = BinaryAddInst (:any) %2: any, %3: any
// CHKDIS-NEXT:  %5 = ReturnInst %4: any
// CHKDIS-NEXT:%BB1:
// CHKDIS-NEXT:  %6 = StoreFrameInst 10: number, [x]: any
// CHKDIS-NEXT:  %7 = StoreFrameInst 1: number, [y]: any
// CHKDIS-NEXT:  %8 = ReturnInst undefined: undefined
// CHKDIS-NEXT:function_end
