/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s

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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %0: any
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): any
// CHECK-NEXT:  %3 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %4 = CallInst [njsf] (:any) %2: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %3: object
// CHECK-NEXT:       StoreStackInst %4: any, %0: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %0: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, C: any, D: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [D]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %C(): any
// CHECK-NEXT:       StoreFrameInst %4: object, [C]: any
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StorePropertyStrictInst %6: object, %4: object, "prototype": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [C]: any
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %D(): any
// CHECK-NEXT:        StoreFrameInst %9: object, [D]: any
// CHECK-NEXT:  %11 = LoadPropertyInst (:object) %8: any, "prototype": string
// CHECK-NEXT:  %12 = AllocObjectInst (:object) 0: number, %11: object
// CHECK-NEXT:        StorePropertyStrictInst %12: object, %9: object, "prototype": string
// CHECK-NEXT:  %14 = LoadFrameInst (:any) [D]: any
// CHECK-NEXT:  %15 = LoadPropertyInst (:any) %14: any, "prototype": string
// CHECK-NEXT:  %16 = AllocObjectInst (:object) 0: number, %15: any
// CHECK-NEXT:  %17 = CallInst (:any) %14: any, %D(): any, empty: any, %14: any, %16: object
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function C(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function D(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [C@""]: any
// CHECK-NEXT:  %2 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %3 = CallInst [njsf] (:any) %1: any, empty: any, empty: any, %2: undefined|object, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
