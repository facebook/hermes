/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -strict -dump-ir -O0 -include-globals=%s.d %s | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -strict -dump-ir -O -include-globals=%s.d %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermesc -hermes-parser -non-strict -dump-ir -O -include-globals=%s.d %s | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-NONSTRICT

// Ensure that global properties are not promoted.
var a = 10;
print(a, process);
process = null;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "a": string
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %2 = StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = StorePropertyStrictInst 10: number, globalObject: object, "a": string
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "process": string
// CHECK-NEXT:  %7 = CallInst (:any) %4: any, empty: any, empty: any, undefined: undefined, %5: any, %6: any
// CHECK-NEXT:  %8 = StoreStackInst %7: any, %1: any
// CHECK-NEXT:  %9 = TryStoreGlobalPropertyStrictInst null: null, globalObject: object, "process": string
// CHECK-NEXT:  %10 = StoreStackInst null: null, %1: any
// CHECK-NEXT:  %11 = LoadStackInst (:any) %1: any
// CHECK-NEXT:  %12 = ReturnInst %11: any
// CHECK-NEXT:function_end

// OPT-CHECK:function global(): null [allCallsitesKnownInStrictMode]
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = DeclareGlobalVarInst "a": string
// OPT-CHECK-NEXT:  %1 = StorePropertyStrictInst 10: number, globalObject: object, "a": string
// OPT-CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "a": string
// OPT-CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "process": string
// OPT-CHECK-NEXT:  %5 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, %3: any, %4: any
// OPT-CHECK-NEXT:  %6 = TryStoreGlobalPropertyStrictInst null: null, globalObject: object, "process": string
// OPT-CHECK-NEXT:  %7 = ReturnInst null: null
// OPT-CHECK-NEXT:function_end

// OPT-NONSTRICT:function global(): null [allCallsitesKnownInStrictMode]
// OPT-NONSTRICT-NEXT:frame = []
// OPT-NONSTRICT-NEXT:%BB0:
// OPT-NONSTRICT-NEXT:  %0 = DeclareGlobalVarInst "a": string
// OPT-NONSTRICT-NEXT:  %1 = StorePropertyLooseInst 10: number, globalObject: object, "a": string
// OPT-NONSTRICT-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NONSTRICT-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "a": string
// OPT-NONSTRICT-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "process": string
// OPT-NONSTRICT-NEXT:  %5 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, %3: any, %4: any
// OPT-NONSTRICT-NEXT:  %6 = StorePropertyLooseInst null: null, globalObject: object, "process": string
// OPT-NONSTRICT-NEXT:  %7 = ReturnInst null: null
// OPT-NONSTRICT-NEXT:function_end
