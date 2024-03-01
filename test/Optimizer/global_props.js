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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "a": string
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %2: any
// CHECK-NEXT:       StorePropertyStrictInst 10: number, globalObject: object, "a": string
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "process": string
// CHECK-NEXT:  %8 = CallInst (:any) %5: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %6: any, %7: any
// CHECK-NEXT:       StoreStackInst %8: any, %2: any
// CHECK-NEXT:        TryStoreGlobalPropertyStrictInst null: null, globalObject: object, "process": string
// CHECK-NEXT:        StoreStackInst null: null, %2: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %2: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// OPT-CHECK:function global(): null
// OPT-CHECK-NEXT:frame = []
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:       DeclareGlobalVarInst "a": string
// OPT-CHECK-NEXT:       StorePropertyStrictInst 10: number, globalObject: object, "a": string
// OPT-CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "a": string
// OPT-CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "process": string
// OPT-CHECK-NEXT:  %5 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: any, %4: any
// OPT-CHECK-NEXT:       TryStoreGlobalPropertyStrictInst null: null, globalObject: object, "process": string
// OPT-CHECK-NEXT:       ReturnInst null: null
// OPT-CHECK-NEXT:function_end

// OPT-NONSTRICT:function global(): null
// OPT-NONSTRICT-NEXT:frame = []
// OPT-NONSTRICT-NEXT:%BB0:
// OPT-NONSTRICT-NEXT:       DeclareGlobalVarInst "a": string
// OPT-NONSTRICT-NEXT:       StorePropertyLooseInst 10: number, globalObject: object, "a": string
// OPT-NONSTRICT-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// OPT-NONSTRICT-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "a": string
// OPT-NONSTRICT-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "process": string
// OPT-NONSTRICT-NEXT:  %5 = CallInst (:any) %2: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: any, %4: any
// OPT-NONSTRICT-NEXT:       StorePropertyLooseInst null: null, globalObject: object, "process": string
// OPT-NONSTRICT-NEXT:       ReturnInst null: null
// OPT-NONSTRICT-NEXT:function_end
