/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

for (const [] = []; ; class C {}) {
  break;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [C: any, ?C.prototype: object, ?C: object]

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = AllocArrayInst (:object) 0: number
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_1_iter: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_2_sourceOrNext: any
// CHECK-NEXT:       StoreStackInst %3: object, %5: any
// CHECK-NEXT:  %7 = IteratorBeginInst (:any) %5: any
// CHECK-NEXT:       StoreStackInst %7: any, %4: any
// CHECK-NEXT:  %9 = AllocStackInst (:any) $?anon_3_iterDone: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %9: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_iterValue: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %9: any
// CHECK-NEXT:        CondBranchInst %12: any, %BB4, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = LoadStackInst (:any) %1: any
// CHECK-NEXT:        ReturnInst %15: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %17 = LoadStackInst (:any) %4: any
// CHECK-NEXT:  %18 = IteratorCloseInst (:any) %17: any, false: boolean
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [this: object]

// CHECK:base constructor C(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: object, "prototype": string
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) %3: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS1.this]: object
// CHECK-NEXT:  %6 = LoadFrameInst (:object) %1: environment, [%VS1.this]: object
// CHECK-NEXT:       ReturnInst %6: object
// CHECK-NEXT:function_end
