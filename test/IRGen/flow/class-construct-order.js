/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Xdump-functions=foo -Werror -typed -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s

function foo(x: number): C {
  // Ensure the target operand of new C() is populated at IRGen.
  return new C(x);
}

class C {
  x: number;
  constructor(x) {
    this.x = x;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function foo(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %0: number, [x]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [C@""]: any
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:object) %2: any, type(object)
// CHECK-NEXT:  %4 = LoadFrameInst (:object) [?C.prototype@""]: object
// CHECK-NEXT:  %5 = UnionNarrowTrustedInst (:object) %4: object
// CHECK-NEXT:  %6 = AllocObjectLiteralInst (:object) "x": string, 0: number
// CHECK-NEXT:       StoreParentInst %5: object, %6: object
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %9 = CheckedTypeCastInst (:number) %8: any, type(number)
// CHECK-NEXT:  %10 = CallInst (:any) %3: object, %C(): any, empty: any, %3: object, %6: object, %9: number
// CHECK-NEXT:        ReturnInst %6: object
// CHECK-NEXT:function_end
