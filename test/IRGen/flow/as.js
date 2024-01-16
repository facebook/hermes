/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions=test_as -Werror -typed -dump-ir -O0 %s | %FileCheckOrRegen %s --match-full-lines

function test_as(): number {
   return (Math.PI as number) * 3.0;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function test_as(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Math": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: any, "PI": string
// CHECK-NEXT:  %2 = CheckedTypeCastInst (:number) %1: any, type(number)
// CHECK-NEXT:  %3 = BinaryMultiplyInst (:any) %2: number, 3: number
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:number) %3: any, type(number)
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:function_end
