/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -hermes-parser -dump-ir %s -non-strict 2>&1 | %FileCheckOrRegen %s --match-full-lines

function one() { return s; return s; }

function two() { return s; return t;}

function three() { return z; return z;}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "one": string
// CHECK-NEXT:       DeclareGlobalVarInst "two": string
// CHECK-NEXT:       DeclareGlobalVarInst "three": string
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %0: environment, %VS0: any, %one(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %4: object, globalObject: object, "one": string
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %0: environment, %VS0: any, %two(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %6: object, globalObject: object, "two": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %VS0: any, %three(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "three": string
// CHECK-NEXT:  %10 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %10: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %10: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:function one(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "s": string
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function two(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "s": string
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function three(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "z": string
// CHECK-NEXT:       ReturnInst %2: any
// CHECK-NEXT:function_end
