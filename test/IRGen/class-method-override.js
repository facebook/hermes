/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -fno-inline -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  override(): number {
    return 1;
  }
  override2(x: number): number|string {
    return 1;
  }
}

class D extends C {
  constructor() {
    'use strict'
    super();
  }
  override(): number {
    return 2;
  }
  override2(x: number|string): number {
    return 2;
  }
}

new D().override();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any
// CHECK-NEXT:frame = [C: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %C(): undefined
// CHECK-NEXT:  %1 = StoreFrameInst %0: object, [C]: object
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %override2(): number
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %override(): number
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) "override": string, %3: object, "override2": string, %2: object
// CHECK-NEXT:  %5 = StorePropertyStrictInst %4: object, %0: object, "prototype": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %D(): undefined
// CHECK-NEXT:  %7 = LoadPropertyInst (:object) %0: object, "prototype": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %"override2 1#"(): number
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %"override 1#"(): number
// CHECK-NEXT:  %10 = AllocObjectLiteralInst (:object) "override": string, %9: object, "override2": string, %8: object
// CHECK-NEXT:  %11 = StoreParentInst %7: object, %10: object
// CHECK-NEXT:  %12 = StorePropertyStrictInst %10: object, %6: object, "prototype": string
// CHECK-NEXT:  %13 = LoadPropertyInst (:any) %6: object, "prototype": string
// CHECK-NEXT:  %14 = AllocObjectInst (:object) 0: number, %13: any
// CHECK-NEXT:  %15 = CallInst (:undefined) %6: object, %D(): undefined, empty: any, undefined: undefined, %14: object
// CHECK-NEXT:  %16 = LoadParentInst (:object) %14: object
// CHECK-NEXT:  %17 = PrLoadInst (:object) %16: object, 0: number, "override": string
// CHECK-NEXT:  %18 = CallInst [njsf] (:any) %17: object, empty: any, empty: any, undefined: undefined, %14: object
// CHECK-NEXT:  %19 = ReturnInst %18: any
// CHECK-NEXT:function_end

// CHECK:function C(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function override2(x: number): number [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function override(): number [typed]
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

// CHECK:function "override2 1#"(x: string|number): number [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 2: number
// CHECK-NEXT:function_end

// CHECK:function "override 1#"(): number [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst 2: number
// CHECK-NEXT:function_end
