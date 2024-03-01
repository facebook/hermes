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
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %inherited(): functionCode
// CHECK-NEXT:  %9 = AllocObjectLiteralInst (:object) "inherited": string, %8: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %9: object, %6: object, "prototype": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [C]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %1: environment, %D(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: object, [D]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:object) %1: environment, [?C.prototype]: object
// CHECK-NEXT:  %17 = PrLoadInst (:object) %16: object, 0: number, "inherited": string
// CHECK-NEXT:  %18 = AllocObjectLiteralInst (:object) "inherited": string, %17: object
// CHECK-NEXT:        StoreParentInst %16: object, %18: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: object, [?D.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %18: object, %14: object, "prototype": string
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [D]: any
// CHECK-NEXT:  %23 = CheckedTypeCastInst (:object) %22: any, type(object)
// CHECK-NEXT:  %24 = LoadFrameInst (:object) %1: environment, [?D.prototype]: object
// CHECK-NEXT:  %25 = UnionNarrowTrustedInst (:object) %24: object
// CHECK-NEXT:  %26 = AllocObjectInst (:object) 0: number, %25: object
// CHECK-NEXT:  %27 = CallInst (:any) %23: object, %D(): functionCode, empty: any, %23: object, %26: object
// CHECK-NEXT:  %28 = LoadParentInst (:object) %26: object
// CHECK-NEXT:  %29 = PrLoadInst (:object) %28: object, 0: number, "inherited": string
// CHECK-NEXT:  %30 = CallInst [njsf] (:any) %29: object, %inherited(): functionCode, empty: any, undefined: undefined, %26: object
// CHECK-NEXT:  %31 = CheckedTypeCastInst (:number) %30: any, type(number)
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
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %inherited(): any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
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
// CHECK-NEXT:  %7 = CallInst (:any) %5: object, empty: any, empty: any, %6: undefined|object, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
