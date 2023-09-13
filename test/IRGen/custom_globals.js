/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir -include-globals %s.d %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir -include-globals %s.d %s -O

var x = CustomGlobalProperty;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "CustomGlobalProperty": string
// CHECK-NEXT:       StorePropertyLooseInst %3: any, globalObject: object, "x": string
// CHECK-NEXT:  %5 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end
