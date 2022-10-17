/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

print(void 0);

print(void "x");

print(void print);

// Auto-generated content below. Please do not modify manually.

// CHECK:function global#0()#1
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %2 = CallInst %1, undefined : undefined, undefined : undefined
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %4 = CallInst %3, undefined : undefined, undefined : undefined
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %7 = UnaryOperatorInst 'void', %6
// CHECK-NEXT:  %8 = CallInst %5, undefined : undefined, %7 : undefined
// CHECK-NEXT:  %9 = ReturnInst %8
// CHECK-NEXT:function_end
