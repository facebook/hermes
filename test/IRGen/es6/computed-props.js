/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen %s --match-full-lines

({
  ['x']: 3,
  get ['y']() {
    return 42;
  },
  set ['y'](val) {},
  ['z']: function() {
    return 100;
  },
});

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:       DefineOwnPropertyInst 3: number, %3: object, "x": string, true: boolean
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %""(): functionCode
// CHECK-NEXT:       DefineOwnGetterSetterInst %5: object, undefined: undefined, %3: object, "y": string, true: boolean
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %" 1#"(): functionCode
// CHECK-NEXT:       DefineOwnGetterSetterInst undefined: undefined, %7: object, %3: object, "y": string, true: boolean
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %" 2#"(): functionCode
// CHECK-NEXT:        DefineOwnPropertyInst %9: object, %3: object, "z": string, true: boolean
// CHECK-NEXT:        StoreStackInst %3: object, %1: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %1: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:function ""(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       ReturnInst 42: number
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [val: any]

// CHECK:function " 1#"(val: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %val: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.val]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function " 2#"(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       ReturnInst 100: number
// CHECK-NEXT:function_end
