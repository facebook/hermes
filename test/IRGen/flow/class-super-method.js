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

  f(): number {
    return this.x * 10;
  }
}

class B extends A {
  constructor(x: number) {
    super(x);
  }

  f(): number {
    return super.f() + 23;
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
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %f(): any
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) "f": string, %6: object
// CHECK-NEXT:       StoreFrameInst %7: object, [?A.prototype]: object
// CHECK-NEXT:       StorePropertyStrictInst %7: object, %4: object, "prototype": string
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [A]: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:object) %10: any, type(object)
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %B(): any
// CHECK-NEXT:        StoreFrameInst %12: object, [B]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:object) [?A.prototype]: object
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %"f 1#"(): any
// CHECK-NEXT:  %16 = AllocObjectLiteralInst (:object) "f": string, %15: object
// CHECK-NEXT:        StoreParentInst %14: object, %16: object
// CHECK-NEXT:        StoreFrameInst %16: object, [?B.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %16: object, %12: object, "prototype": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function A(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: number, [x]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:number) %3: any, type(number)
// CHECK-NEXT:       PrStoreInst %4: number, %0: object, 0: number, "x": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = PrLoadInst (:number) %0: object, 0: number, "x": string
// CHECK-NEXT:  %2 = BinaryMultiplyInst (:any) %1: number, 10: number
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:number) %2: any, type(number)
// CHECK-NEXT:       ReturnInst %3: number
// CHECK-NEXT:function_end

// CHECK:function B(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: number, [x]: any
// CHECK-NEXT:  %3 = LoadFrameInst (:any) [A@""]: any
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:object) %3: any, type(object)
// CHECK-NEXT:  %5 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [x]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:number) %6: any, type(number)
// CHECK-NEXT:  %8 = CallInst [njsf] (:any) %4: object, empty: any, empty: any, %5: undefined|object, %0: object, %7: number
// CHECK-NEXT:  %9 = CheckedTypeCastInst (:undefined) %8: any, type(undefined)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function "f 1#"(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadFrameInst (:object) [?A.prototype@""]: object
// CHECK-NEXT:  %2 = PrLoadInst (:object) %1: object, 0: number, "f": string
// CHECK-NEXT:  %3 = CallInst [njsf] (:any) %2: object, empty: any, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:number) %3: any, type(number)
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: number, 23: number
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:number) %5: any, type(number)
// CHECK-NEXT:       ReturnInst %6: number
// CHECK-NEXT:function_end
