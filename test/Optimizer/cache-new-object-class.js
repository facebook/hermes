/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O -fno-inline | %FileCheckOrRegen --match-full-lines %s

class A {
  f1 = 10;
  f2;
  f3 = "test";
}

new A();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [<instElemInitFunc:A>: object]

// CHECK:function global(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %<instance_members_initializer:A>(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: object, [%VS0.<instElemInitFunc:A>]: object
// CHECK-NEXT:  %3 = AllocStackInst (:object) $?anon_1_clsPrototype: any
// CHECK-NEXT:  %4 = CreateClassInst (:object) %0: environment, %VS0: any, %A(): functionCode, empty: any, %3: object
// CHECK-NEXT:  %5 = CreateThisInst (:object) %4: object, %4: object
// CHECK-NEXT:  %6 = CallInst (:undefined) %4: object, %A(): functionCode, true: boolean, %0: environment, %4: object, %5: object
// CHECK-NEXT:       ReturnInst %5: object
// CHECK-NEXT:function_end

// CHECK:function <instance_members_initializer:A>(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       CacheNewObjectInst %0: any, %1: undefined|object, "f1": string, "f2": string, "f3": string
// CHECK-NEXT:       DefineOwnPropertyInst 10: number, %0: any, "f1": string, true: boolean
// CHECK-NEXT:       DefineOwnPropertyInst undefined: undefined, %0: any, "f2": string, true: boolean
// CHECK-NEXT:       DefineOwnPropertyInst "test": string, %0: any, "f3": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:base constructor A(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:object) %1: environment, [%VS0.<instElemInitFunc:A>]: object
// CHECK-NEXT:  %3 = CallInst (:undefined) %2: object, %<instance_members_initializer:A>(): functionCode, true: boolean, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
