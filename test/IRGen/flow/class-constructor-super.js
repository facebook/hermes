/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s

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
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %""(): functionCode
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
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %4: object, [C]: any
// CHECK-NEXT:  %6 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StoreFrameInst %6: object, [?C.prototype]: object
// CHECK-NEXT:       StorePropertyStrictInst %6: object, %4: object, "prototype": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) [C]: any
// CHECK-NEXT:  %10 = CheckedTypeCastInst (:object) %9: any, type(object)
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %D(): functionCode
// CHECK-NEXT:        StoreFrameInst %11: object, [D]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:object) [?C.prototype]: object
// CHECK-NEXT:  %14 = AllocObjectInst (:object) 0: number, %13: object
// CHECK-NEXT:        StoreFrameInst %14: object, [?D.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %14: object, %11: object, "prototype": string
// CHECK-NEXT:  %17 = LoadFrameInst (:any) [D]: any
// CHECK-NEXT:  %18 = CheckedTypeCastInst (:object) %17: any, type(object)
// CHECK-NEXT:  %19 = LoadFrameInst (:object) [?D.prototype]: object
// CHECK-NEXT:  %20 = UnionNarrowTrustedInst (:object) %19: object
// CHECK-NEXT:  %21 = AllocObjectInst (:object) 0: number, %20: object
// CHECK-NEXT:  %22 = CallInst (:any) %18: object, %D(): functionCode, empty: any, %18: object, %21: object
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
// CHECK-NEXT:  %2 = CheckedTypeCastInst (:object) %1: any, type(object)
// CHECK-NEXT:  %3 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %4 = CallInst [njsf] (:any) %2: object, empty: any, empty: any, %3: undefined|object, %0: object
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:undefined) %4: any, type(undefined)
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
