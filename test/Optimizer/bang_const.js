/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s

print(!0);

print(!-0);

print(!0.0);

print(!NaN);

print(!1);

print(!null);

print(!undefined);

print(!"");

print(!"abc");

print(!print);

print(!true);

print(!false);

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %5 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "NaN": string
// CHECK-NEXT:  %8 = UnaryBangInst (:boolean) %7: any
// CHECK-NEXT:  %9 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %8: boolean
// CHECK-NEXT:  %10 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %11 = CallInst (:any) %10: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %12 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %13 = CallInst (:any) %12: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %14 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %15 = CallInst (:any) %14: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %16 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %17 = CallInst (:any) %16: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %18 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %19 = CallInst (:any) %18: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %20 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %21 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %22 = UnaryBangInst (:boolean) %21: any
// CHECK-NEXT:  %23 = CallInst (:any) %20: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %22: boolean
// CHECK-NEXT:  %24 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %25 = CallInst (:any) %24: any, empty: any, empty: any, undefined: undefined, undefined: undefined, false: boolean
// CHECK-NEXT:  %26 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %27 = CallInst (:any) %26: any, empty: any, empty: any, undefined: undefined, undefined: undefined, true: boolean
// CHECK-NEXT:  %28 = ReturnInst %27: any
// CHECK-NEXT:function_end
