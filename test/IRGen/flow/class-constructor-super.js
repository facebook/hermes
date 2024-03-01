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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:function ""(exports: any): any
// CHECK-NEXT:frame = [exports: any, C: any, D: any, ?C.prototype: object, ?D.prototype: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [C]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [D]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [C]: any
// CHECK-NEXT:  %8 = AllocObjectInst (:object) 0: number, empty: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %8: object, %6: object, "prototype": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [C]: any
// CHECK-NEXT:  %12 = CheckedTypeCastInst (:object) %11: any, type(object)
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %1: environment, %D(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [D]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:object) %1: environment, [?C.prototype]: object
// CHECK-NEXT:  %16 = AllocObjectInst (:object) 0: number, %15: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: object, [?D.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %16: object, %13: object, "prototype": string
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [D]: any
// CHECK-NEXT:  %20 = CheckedTypeCastInst (:object) %19: any, type(object)
// CHECK-NEXT:  %21 = LoadFrameInst (:object) %1: environment, [?D.prototype]: object
// CHECK-NEXT:  %22 = UnionNarrowTrustedInst (:object) %21: object
// CHECK-NEXT:  %23 = AllocObjectInst (:object) 0: number, %22: object
// CHECK-NEXT:  %24 = CallInst (:any) %20: object, %D(): functionCode, empty: any, %20: object, %23: object
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function C(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %C(): any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function D(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %D(): any, %1: environment
// CHECK-NEXT:  %3 = ResolveScopeInst (:environment) %""(): any, %2: environment
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %3: environment, [C@""]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:object) %4: any, type(object)
// CHECK-NEXT:  %6 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %7 = CallInst [njsf] (:any) %5: object, empty: any, empty: any, %6: undefined|object, %0: object
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:undefined) %7: any, type(undefined)
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
