/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -fno-inline -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

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
// CHECK-NEXT:frame = [exports: any, C: any, D: any, ?C.prototype: object, ?D.prototype: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %0: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [C]: any
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [D]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %C(): any
// CHECK-NEXT:       StoreFrameInst %4: object, [C]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %inherited(): any
// CHECK-NEXT:  %7 = AllocObjectLiteralInst (:object) "inherited": string, %6: object
// CHECK-NEXT:       StoreFrameInst %7: object, [?C.prototype]: object
// CHECK-NEXT:       StorePropertyStrictInst %7: object, %4: object, "prototype": string
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [C]: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:object) %10: any, type(object)
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %D(): any
// CHECK-NEXT:        StoreFrameInst %12: object, [D]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:object) [?C.prototype]: object
// CHECK-NEXT:  %15 = PrLoadInst (:object) %14: object, 0: number, "inherited": string
// CHECK-NEXT:  %16 = AllocObjectLiteralInst (:object) "inherited": string, %15: object
// CHECK-NEXT:        StoreParentInst %14: object, %16: object
// CHECK-NEXT:        StoreFrameInst %16: object, [?D.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %16: object, %12: object, "prototype": string
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [D]: any
// CHECK-NEXT:  %21 = CheckedTypeCastInst (:object) %20: any, type(object)
// CHECK-NEXT:  %22 = LoadFrameInst (:object) [?D.prototype]: object
// CHECK-NEXT:  %23 = UnionNarrowTrustedInst (:object) %22: object
// CHECK-NEXT:  %24 = AllocObjectInst (:object) 0: number, %23: object
// CHECK-NEXT:  %25 = CallInst (:any) %21: object, %D(): any, empty: any, %21: object, %24: object
// CHECK-NEXT:  %26 = LoadParentInst (:object) %24: object
// CHECK-NEXT:  %27 = PrLoadInst (:object) %26: object, 0: number, "inherited": string
// CHECK-NEXT:  %28 = CallInst [njsf] (:any) %27: object, empty: any, empty: any, undefined: undefined, %24: object
// CHECK-NEXT:  %29 = CheckedTypeCastInst (:number) %28: any, type(number)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function C(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function inherited(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function D(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = LoadFrameInst (:any) [C@""]: any
// CHECK-NEXT:  %2 = CheckedTypeCastInst (:object) %1: any, type(object)
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %4 = CallInst (:any) %2: object, empty: any, empty: any, %3: undefined|object, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
