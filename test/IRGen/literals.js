/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function foo() {
  return "hi"
  return 2.312
  return 12
  return 0x12
  return true
  return undefined
  return null
}

foo()

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreStackInst %7: any, %4: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %4: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:function foo(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       ReturnInst "hi": string
// CHECK-NEXT:function_end
