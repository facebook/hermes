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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any, C: any, ?C.prototype: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C]: any
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %1: environment, %VS1: any, %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS1.C]: any
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %1: environment, %VS1: any, %method(): functionCode
// CHECK-NEXT:  %8 = AllocObjectLiteralInst (:object) empty: any, "method": string, %7: object
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [%VS1.?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %8: object, %5: object, "prototype": string
// CHECK-NEXT:  %11 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.C]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = LoadFrameInst (:object) %1: environment, [%VS1.?C.prototype]: object
// CHECK-NEXT:  %15 = UnionNarrowTrustedInst (:object) %14: object
// CHECK-NEXT:  %16 = AllocObjectLiteralInst (:object) %15: object
// CHECK-NEXT:  %17 = TypedLoadParentInst (:object) %16: object
// CHECK-NEXT:  %18 = PrLoadInst (:object) %17: object, 0: number, "method": string
// CHECK-NEXT:  %19 = CallInst [njsf] (:any) %18: object, %method(): functionCode, true: boolean, empty: any, undefined: undefined, %16: object
// CHECK-NEXT:  %20 = CheckedTypeCastInst (:number) %19: any, type(number)
// CHECK-NEXT:  %21 = CallInst (:any) %11: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %20: number
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function C(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function method(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end
