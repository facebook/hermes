/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  constructor() {
  }
}

class D extends C {
  constructor() {
    super();
  }
}

new D();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %""(): undefined
// CHECK-NEXT:  %1 = CallInst [njsf] (:undefined) %0: object, %""(): undefined, empty: any, undefined: undefined, 0: number, 0: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function ""(exports: number): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:frame = [C: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %C(): undefined
// CHECK-NEXT:       StoreFrameInst %0: object, [C]: object
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StorePropertyStrictInst %2: object, %0: object, "prototype": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %D(): undefined
// CHECK-NEXT:  %5 = LoadPropertyInst (:object) %0: object, "prototype": string
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 0: number, %5: object
// CHECK-NEXT:       StorePropertyStrictInst %6: object, %4: object, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %4: object, "prototype": string
// CHECK-NEXT:  %9 = AllocObjectInst (:object) 0: number, %8: any
// CHECK-NEXT:  %10 = CallInst (:undefined) %4: object, %D(): undefined, empty: any, undefined: undefined, %9: object
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function C(): undefined [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function D(): undefined [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadFrameInst (:object) [C@""]: object
// CHECK-NEXT:  %2 = CallInst [njsf] (:undefined) %1: object, %C(): undefined, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
