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
// CHECK-NEXT:frame = [exports: any, C: any, D: any, foo: any, ?C.prototype: object, ?D.prototype: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [C]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [D]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [foo]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [C]: any
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %1: environment, %override(): functionCode
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %1: environment, %override2(): functionCode
// CHECK-NEXT:  %12 = AllocObjectLiteralInst (:object) "override": string, %10: object, "override2": string, %11: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: object, [?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %12: object, %8: object, "prototype": string
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [C]: any
// CHECK-NEXT:  %16 = CheckedTypeCastInst (:object) %15: any, type(object)
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %1: environment, %D(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: object, [D]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:object) %1: environment, [?C.prototype]: object
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %1: environment, %"override 1#"(): functionCode
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %1: environment, %"override2 1#"(): functionCode
// CHECK-NEXT:  %22 = AllocObjectLiteralInst (:object) "override": string, %20: object, "override2": string, %21: object
// CHECK-NEXT:        StoreParentInst %19: object, %22: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %22: object, [?D.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %22: object, %17: object, "prototype": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(c: object, d: object): any [typed]
// CHECK-NEXT:frame = [c: any, d: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %foo(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:object) %c: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [c]: any
// CHECK-NEXT:  %4 = LoadParamInst (:object) %d: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [d]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [c]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:object) %6: any, type(object)
// CHECK-NEXT:  %8 = LoadParentInst (:object) %7: object
// CHECK-NEXT:  %9 = PrLoadInst (:object) %8: object, 0: number, "override": string
// CHECK-NEXT:  %10 = CallInst [njsf] (:any) %9: object, empty: any, empty: any, undefined: undefined, %7: object
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:number) %10: any, type(number)
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [d]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = LoadParentInst (:object) %13: object
// CHECK-NEXT:  %15 = PrLoadInst (:object) %14: object, 0: number, "override": string
// CHECK-NEXT:  %16 = CallInst [njsf] (:any) %15: object, %"override 1#"(): functionCode, empty: any, undefined: undefined, %13: object
// CHECK-NEXT:  %17 = CheckedTypeCastInst (:number) %16: any, type(number)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function C(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function override(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %override(): any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:function override2(x: number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %override2(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [x]: any
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

// CHECK:function "override 1#"(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"override 1#"(): any, %0: environment
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end

// CHECK:function "override2 1#"(x: string|number): any [typed]
// CHECK-NEXT:frame = [x: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %"override2 1#"(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:string|number) %x: string|number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: string|number, [x]: any
// CHECK-NEXT:       ReturnInst 2: number
// CHECK-NEXT:function_end
