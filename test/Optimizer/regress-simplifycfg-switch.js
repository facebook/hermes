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
// CHKUNOPT-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHKUNOPT-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHKUNOPT-NEXT:       SwitchInst -0: number, %BB3, 0: number, %BB2
// CHKUNOPT-NEXT:%BB1:
// CHKUNOPT-NEXT:       SwitchInst NaN: number, %BB6, NaN: number, %BB5
// CHKUNOPT-NEXT:%BB2:
// CHKUNOPT-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHKUNOPT-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "0": string
// CHKUNOPT-NEXT:       StoreStackInst %5: any, %0: any
// CHKUNOPT-NEXT:       BranchInst %BB1
// CHKUNOPT-NEXT:%BB3:
// CHKUNOPT-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHKUNOPT-NEXT:  %9 = CallInst (:any) %8: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "default": string
// CHKUNOPT-NEXT:        StoreStackInst %9: any, %0: any
// CHKUNOPT-NEXT:        BranchInst %BB1
// CHKUNOPT-NEXT:%BB4:
// CHKUNOPT-NEXT:        StoreStackInst 0: number, %0: any
// CHKUNOPT-NEXT:  %13 = LoadStackInst (:any) %0: any
// CHKUNOPT-NEXT:        ReturnInst %13: any
// CHKUNOPT-NEXT:%BB5:
// CHKUNOPT-NEXT:  %15 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHKUNOPT-NEXT:  %16 = CallInst (:any) %15: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "NaN": string
// CHKUNOPT-NEXT:        StoreStackInst %16: any, %0: any
// CHKUNOPT-NEXT:        BranchInst %BB4
// CHKUNOPT-NEXT:%BB6:
// CHKUNOPT-NEXT:  %19 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHKUNOPT-NEXT:  %20 = CallInst (:any) %19: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "default": string
// CHKUNOPT-NEXT:        StoreStackInst %20: any, %0: any
// CHKUNOPT-NEXT:        BranchInst %BB4
// CHKUNOPT-NEXT:function_end

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "0": string
// CHECK-NEXT:       StoreStackInst %3: any, %0: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined, "default": string
// CHECK-NEXT:       StoreStackInst %6: any, %0: any
// CHECK-NEXT:       StoreStackInst 0: number, %0: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %0: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end
