/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class A {
  x: number;

  constructor(x: number) {
    this.x = x;
  }
}

class B extends A {
  constructor(x: number) {
    super(x);
  }

  f(): number {
    return super.x;
  }
}

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
// CHECK-NEXT:frame = [exports: any, A: any, B: any, ?A.prototype: object, ?B.prototype: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [A]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [B]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %A(): any
// CHECK-NEXT:       StoreFrameInst %4: object, [A]: any
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StoreFrameInst %6: object, [?A.prototype]: object
// CHECK-NEXT:       StorePropertyStrictInst %6: object, %4: object, "prototype": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [A]: any
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %B(): any
// CHECK-NEXT:        StoreFrameInst %10: object, [B]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:object) [?A.prototype]: object
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %f(): any
// CHECK-NEXT:  %14 = AllocObjectLiteralInst (:object) "f": string, %13: object
// CHECK-NEXT:        StoreParentInst %12: object, %14: object
// CHECK-NEXT:        StoreFrameInst %14: object, [?B.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %14: object, %10: object, "prototype": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function A(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: number, [x]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:       PrStoreInst %3: any, %0: object, 0: number, "x": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function B(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: number, [x]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [A@""]: any
// CHECK-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %5 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %6 = CallInst [njsf] (:any) %3: any, empty: any, empty: any, %4: undefined|object, %0: object, %5: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = PrLoadInst (:number) %0: object, 0: number, "x": string
// CHECK-NEXT:       ReturnInst %1: number
// CHECK-NEXT:function_end
