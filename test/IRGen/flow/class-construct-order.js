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

// CHECK:scope %VS0 [exports: any, foo: any, C: any, ?C.prototype: object]

// CHECK:scope %VS1 [x: any]

// CHECK:function foo(x: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [%VS1.x]: any
// CHECK-NEXT:  %4 = ResolveScopeInst (:environment) %VS0: any, %VS1: any, %1: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %4: environment, [%VS0.C]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:object) %5: any, type(object)
// CHECK-NEXT:  %7 = ResolveScopeInst (:environment) %VS0: any, %VS1: any, %1: environment
// CHECK-NEXT:  %8 = LoadFrameInst (:object) %7: environment, [%VS0.?C.prototype]: object
// CHECK-NEXT:  %9 = UnionNarrowTrustedInst (:object) %8: object
// CHECK-NEXT:  %10 = AllocObjectLiteralInst (:object) "x": string, 0: number
// CHECK-NEXT:        StoreParentInst %9: object, %10: object
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.x]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:number) %12: any, type(number)
// CHECK-NEXT:  %14 = CallInst (:any) %6: object, %C(): functionCode, empty: any, %6: object, %10: object, %13: number
// CHECK-NEXT:        ReturnInst %10: object
// CHECK-NEXT:function_end
