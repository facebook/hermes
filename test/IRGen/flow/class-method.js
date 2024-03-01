/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  method(): number {
    return 1;
  }
}

print(new C().method());

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
// CHECK-NEXT:frame = [exports: any, C: any, ?C.prototype: object]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %""(): any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [C]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [C]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %method(): functionCode
// CHECK-NEXT:  %8 = AllocObjectLiteralInst (:object) "method": string, %7: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %8: object, %5: object, "prototype": string
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [C]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = LoadFrameInst (:object) %1: environment, [?C.prototype]: object
// CHECK-NEXT:  %15 = UnionNarrowTrustedInst (:object) %14: object
// CHECK-NEXT:  %16 = AllocObjectInst (:object) 0: number, %15: object
// CHECK-NEXT:  %17 = LoadParentInst (:object) %16: object
// CHECK-NEXT:  %18 = PrLoadInst (:object) %17: object, 0: number, "method": string
// CHECK-NEXT:  %19 = CallInst [njsf] (:any) %18: object, %method(): functionCode, empty: any, undefined: undefined, %16: object
// CHECK-NEXT:  %20 = CheckedTypeCastInst (:number) %19: any, type(number)
// CHECK-NEXT:  %21 = CallInst (:any) %11: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %20: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function C(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function method(): any [typed]
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %""(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %method(): any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end
