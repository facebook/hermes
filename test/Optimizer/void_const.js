/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

print(void 0);

print(void "x");

print(void print);

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %1 = CallInst %0, undefined : undefined, undefined : undefined
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %3 = CallInst %2, undefined : undefined, undefined : undefined
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
// CHECK-NEXT:  %6 = UnaryOperatorInst 'void', %5
// CHECK-NEXT:  %7 = CallInst %4, undefined : undefined, %6 : undefined
// CHECK-NEXT:  %8 = ReturnInst %7
// CHECK-NEXT:function_end
