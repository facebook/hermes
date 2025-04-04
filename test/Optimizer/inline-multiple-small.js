/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict'

function outer(a, b) {
  function f1() {
    return 123;
  }
  function f2(x, y, z) {
    return x * y * z * y * z;
  }
  // Don't inline this because it's complicated.
  function f3(x) {
    return globalThis.foo + x * x + x * x + x * x;
  }
  return f1() + f1() + f2(2, 3, 4) + f2(3, 4, 5) + f3(10) + f3(100);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %outer(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %1: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function outer(a: any, b: any): string|number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: any, "foo": string
// CHECK-NEXT:  %2 = BinaryAddInst (:string|number) %1: any, 100: number
// CHECK-NEXT:  %3 = BinaryAddInst (:string|number) %2: string|number, 100: number
// CHECK-NEXT:  %4 = BinaryAddInst (:string|number) %3: string|number, 100: number
// CHECK-NEXT:  %5 = BinaryAddInst (:string|number) 1734: number, %4: string|number
// CHECK-NEXT:  %6 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "foo": string
// CHECK-NEXT:  %8 = BinaryAddInst (:string|number) %7: any, 10000: number
// CHECK-NEXT:  %9 = BinaryAddInst (:string|number) %8: string|number, 10000: number
// CHECK-NEXT:  %10 = BinaryAddInst (:string|number) %9: string|number, 10000: number
// CHECK-NEXT:  %11 = BinaryAddInst (:string|number) %5: string|number, %10: string|number
// CHECK-NEXT:        ReturnInst %11: string|number
// CHECK-NEXT:function_end
