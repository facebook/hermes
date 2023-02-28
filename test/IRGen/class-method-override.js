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
  constructor() {
    super();
  }
  override(): number {
    return 2;
  }
}

new D().override();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [C: closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %C(): undefined
// CHECK-NEXT:  %1 = StoreFrameInst %0: closure, [C]: closure
// CHECK-NEXT:  %2 = CreateFunctionInst (:closure) %override(): number
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) "override": string, %2: closure
// CHECK-NEXT:  %4 = StorePropertyStrictInst %3: object, %0: closure, "prototype": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:closure) %D(): undefined
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %0: closure, "prototype": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %"override 1#"(): number
// CHECK-NEXT:  %8 = AllocObjectLiteralInst (:object) "override": string, %7: closure
// CHECK-NEXT:  %9 = CallBuiltinInst (:any) [HermesBuiltin.silentSetPrototypeOf]: number, empty: any, empty: any, undefined: undefined, %8: object, %6: any
// CHECK-NEXT:  %10 = StorePropertyStrictInst %8: object, %5: closure, "prototype": string
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) %5: closure, "prototype": string
// CHECK-NEXT:  %12 = AllocObjectInst (:object) 0: number, %11: any
// CHECK-NEXT:  %13 = ConstructInst (:undefined) %5: closure, %D(): undefined, empty: any, %12: object
// CHECK-NEXT:  %14 = LoadParentInst (:object) %12: object
// CHECK-NEXT:  %15 = PrLoadInst (:closure) %14: object, 0: number, "override": string
// CHECK-NEXT:  %16 = CallInst (:any) %15: closure, empty: any, empty: any, %12: object
// CHECK-NEXT:  %17 = ReturnInst %16: any
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
// CHECK-NEXT:  %0 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %1 = LoadFrameInst (:closure) [C@global]: closure
// CHECK-NEXT:  %2 = CallInst (:any) %1: closure, %C(): undefined, empty: any, %0: any
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "override 1#"(): number
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 2: number
// CHECK-NEXT:function_end
