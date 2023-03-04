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

// CHECK:function global(): object
// CHECK-NEXT:frame = [C: closure]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:closure) %C(): undefined
// CHECK-NEXT:  %1 = StoreFrameInst %0: closure, [C]: closure
// CHECK-NEXT:  %2 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %3 = StorePropertyStrictInst %2: object, %0: closure, "prototype": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:closure) %D(): undefined
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %0: closure, "prototype": string
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 0: number, %5: any
// CHECK-NEXT:  %7 = StorePropertyStrictInst %6: object, %4: closure, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %4: closure, "prototype": string
// CHECK-NEXT:  %9 = AllocObjectInst (:object) 0: number, %8: any
// CHECK-NEXT:  %10 = ConstructInst (:undefined) %4: closure, %D(): undefined, empty: any, %9: object
// CHECK-NEXT:  %11 = ReturnInst %9: object
// CHECK-NEXT:function_end

// CHECK:function C(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function D(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %this: any
// CHECK-NEXT:  %1 = LoadFrameInst (:closure) [C@global]: closure
// CHECK-NEXT:  %2 = CallInst (:any) %1: closure, %C(): undefined, empty: any, %0: any
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
