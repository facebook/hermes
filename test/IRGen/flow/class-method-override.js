/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -fno-inline -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

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

function foo(c: C, d: D){
  c.override();
  d.override();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any, C: any, D: any, foo: any, ?C.prototype: object, ?D.prototype: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.D]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.foo]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [%VS1.C]: any
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %1: environment, %override(): functionCode
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %1: environment, %override2(): functionCode
// CHECK-NEXT:  %12 = AllocObjectLiteralInst (:object) empty: any, "override": string, %10: object, "override2": string, %11: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: object, [%VS1.?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %12: object, %8: object, "prototype": string
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [%VS1.C]: any
// CHECK-NEXT:  %16 = CheckedTypeCastInst (:object) %15: any, type(object)
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %1: environment, %D(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: object, [%VS1.D]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:object) %1: environment, [%VS1.?C.prototype]: object
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %1: environment, %"override 1#"(): functionCode
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %1: environment, %"override2 1#"(): functionCode
// CHECK-NEXT:  %22 = AllocObjectLiteralInst (:object) empty: any, "override": string, %20: object, "override2": string, %21: object
// CHECK-NEXT:        TypedStoreParentInst %19: object, %22: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %22: object, [%VS1.?D.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %22: object, %17: object, "prototype": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [c: any, d: any]

// CHECK:function foo(c: object, d: object): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:object) %c: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS2.c]: any
// CHECK-NEXT:  %4 = LoadParamInst (:object) %d: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS2.d]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS2.c]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:object) %6: any, type(object)
// CHECK-NEXT:  %8 = TypedLoadParentInst (:object) %7: object
// CHECK-NEXT:  %9 = PrLoadInst (:object) %8: object, 0: number, "override": string
// CHECK-NEXT:  %10 = CallInst [njsf] (:any) %9: object, empty: any, false: boolean, empty: any, undefined: undefined, %7: object
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:number) %10: any, type(number)
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS2.d]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = TypedLoadParentInst (:object) %13: object
// CHECK-NEXT:  %15 = PrLoadInst (:object) %14: object, 0: number, "override": string
// CHECK-NEXT:  %16 = CallInst [njsf] (:any) %15: object, %"override 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, %13: object
// CHECK-NEXT:  %17 = CheckedTypeCastInst (:number) %16: any, type(number)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function C(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:function override(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [x: any]

// CHECK:function override2(x: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [%VS5.x]: any
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:scope %VS6 []

// CHECK:derived constructor D(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS6: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %1: environment, [%VS1.C]: any
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:object) %3: any, type(object)
// CHECK-NEXT:  %5 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %6 = CallInst (:any) %4: object, empty: any, false: boolean, empty: any, %5: object, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS7 []

// CHECK:function "override 1#"(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end

// CHECK:scope %VS8 [x: any]

// CHECK:function "override2 1#"(x: string|number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS8: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:string|number) %x: string|number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: string|number, [%VS8.x]: any
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end
