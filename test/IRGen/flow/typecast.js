/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-ir -O0 %s | %FileCheckOrRegen %s --match-full-lines

(function(){
  return (Math.PI : number) * 3.0;
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %" 1#"(): functionCode
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %4: object, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function " 1#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "Math": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) %2: any, "PI": string
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:number) %3: any, type(number)
// CHECK-NEXT:  %5 = BinaryMultiplyInst (:any) %4: number, 3: number
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:number) %5: any, type(number)
// CHECK-NEXT:       ReturnInst %6: number
// CHECK-NEXT:function_end
