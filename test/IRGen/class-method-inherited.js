/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-inline -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  inherited(): number {
    return 1;
  }
}

class D extends C {
  constructor() {
    super();
  }
}

new D().inherited();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = [C: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %C(): undefined
// CHECK-NEXT:  %1 = StoreFrameInst %0: object, [C]: object
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %inherited(): number
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) "inherited": string, %2: object
// CHECK-NEXT:  %4 = StorePropertyStrictInst %3: object, %0: object, "prototype": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %D(): undefined
// CHECK-NEXT:  %6 = LoadPropertyInst (:object) %0: object, "prototype": string
// CHECK-NEXT:  %7 = PrLoadInst (:object) %6: object, 0: number, "inherited": string
// CHECK-NEXT:  %8 = AllocObjectLiteralInst (:object) "inherited": string, %7: object
// CHECK-NEXT:  %9 = StoreParentInst %6: object, %8: object
// CHECK-NEXT:  %10 = StorePropertyStrictInst %8: object, %5: object, "prototype": string
// CHECK-NEXT:  %11 = LoadPropertyInst (:any) %5: object, "prototype": string
// CHECK-NEXT:  %12 = AllocObjectInst (:object) 0: number, %11: any
// CHECK-NEXT:  %13 = CallInst (:undefined) %5: object, %D(): undefined, empty: any, undefined: undefined, %12: object
// CHECK-NEXT:  %14 = LoadParentInst (:object) %12: object
// CHECK-NEXT:  %15 = PrLoadInst (:object) %14: object, 0: number, "inherited": string
// CHECK-NEXT:  %16 = CallInst [njsf] (:any) %15: object, empty: any, empty: any, undefined: undefined, %12: object
// CHECK-NEXT:  %17 = ReturnInst %16: any
// CHECK-NEXT:function_end

// CHECK:function C(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function inherited(): number [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function D(): undefined [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadFrameInst (:object) [C@global]: object
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %C(): undefined, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:  %3 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end
