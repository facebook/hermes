/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=CHKUNOPT
// RUN: %shermes -Xcustom-opt=simplifycfg -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

// Test for a bug in SimplifyCFG, where strict equality comparison was
// incorrect for numbers. -0 and +0 were considered non-equal, while
// NaN and NaN were considered equal.

switch (-0) {
  case 0:
    // Correct output.
    print("0")
    break;
  default:
    print("default")
    break;
}

switch (0/0) {
  case 0/0:
    print("NaN")
    break;
  default:
    // Correct output.
    print("default")
    break;
}

0;

// Auto-generated content below. Please do not modify manually.

// CHKUNOPT:function global(): any
// CHKUNOPT-NEXT:frame = []
// CHKUNOPT-NEXT:%BB0:
// CHKUNOPT-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHKUNOPT-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHKUNOPT-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHKUNOPT-NEXT:       SwitchInst -0: number, %BB3, 0: number, %BB2
// CHKUNOPT-NEXT:%BB1:
// CHKUNOPT-NEXT:       SwitchInst NaN: number, %BB6, NaN: number, %BB5
// CHKUNOPT-NEXT:%BB2:
// CHKUNOPT-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHKUNOPT-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "0": string
// CHKUNOPT-NEXT:       StoreStackInst %6: any, %1: any
// CHKUNOPT-NEXT:       BranchInst %BB1
// CHKUNOPT-NEXT:%BB3:
// CHKUNOPT-NEXT:  %9 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHKUNOPT-NEXT:  %10 = CallInst (:any) %9: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "default": string
// CHKUNOPT-NEXT:        StoreStackInst %10: any, %1: any
// CHKUNOPT-NEXT:        BranchInst %BB1
// CHKUNOPT-NEXT:%BB4:
// CHKUNOPT-NEXT:        StoreStackInst 0: number, %1: any
// CHKUNOPT-NEXT:  %14 = LoadStackInst (:any) %1: any
// CHKUNOPT-NEXT:        ReturnInst %14: any
// CHKUNOPT-NEXT:%BB5:
// CHKUNOPT-NEXT:  %16 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHKUNOPT-NEXT:  %17 = CallInst (:any) %16: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "NaN": string
// CHKUNOPT-NEXT:        StoreStackInst %17: any, %1: any
// CHKUNOPT-NEXT:        BranchInst %BB4
// CHKUNOPT-NEXT:%BB6:
// CHKUNOPT-NEXT:  %20 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHKUNOPT-NEXT:  %21 = CallInst (:any) %20: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "default": string
// CHKUNOPT-NEXT:        StoreStackInst %21: any, %1: any
// CHKUNOPT-NEXT:        BranchInst %BB4
// CHKUNOPT-NEXT:function_end

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "0": string
// CHECK-NEXT:       StoreStackInst %4: any, %1: any
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "default": string
// CHECK-NEXT:       StoreStackInst %7: any, %1: any
// CHECK-NEXT:       StoreStackInst 0: number, %1: any
// CHECK-NEXT:  %10 = LoadStackInst (:any) %1: any
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end
