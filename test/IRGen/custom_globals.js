/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir -include-globals %s.d %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir -include-globals %s.d %s -O

var x = CustomGlobalProperty;

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "x" : string
// CHECK-NEXT:  %1 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %2 = StoreStackInst undefined : undefined, %1
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "CustomGlobalProperty" : string
// CHECK-NEXT:  %4 = StorePropertyLooseInst %3, globalObject : object, "x" : string
// CHECK-NEXT:  %5 = LoadStackInst %1
// CHECK-NEXT:  %6 = ReturnInst %5
// CHECK-NEXT:function_end
