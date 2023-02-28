/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  override(): number {
    return 1;
  }
}

class D extends C {
  override(): number {
    return 2;
  }
}

new D().override();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %C(): undefined
// CHECK-NEXT:  %1 = CreateFunctionInst (:closure) %override(): number
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) "override": string, %1: closure
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2: object, %0: closure, "prototype": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %D(): undefined
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %0: closure, "prototype": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:closure) %"override 1#"(): number
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) "override": string, %6: closure
// CHECK-NEXT:  %8 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, %7: object, %5: any
// CHECK-NEXT:  %9 = StorePropertyStrictInst %7: object, %4: closure, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %4: closure, "prototype": string
// CHECK-NEXT:  %11 = AllocObjectInst (:object) 0: number, %10: any
// CHECK-NEXT:  %12 = LoadParentInst (:object) %11: object
// CHECK-NEXT:  %13 = PrLoadInst (:closure) %12: object, 0: number, "override": string
// CHECK-NEXT:  %14 = CallInst (:any) %13: closure, empty: any, empty: any, %11: object
// CHECK-NEXT:  %15 = ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:function C(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function override(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function D(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "override 1#"(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 2: number
// CHECK-NEXT:function_end
