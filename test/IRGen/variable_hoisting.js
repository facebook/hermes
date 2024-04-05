/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

main()

function main() {

  function foo(x) { return capture_me; }

  var capture_me;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %main(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "main": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) globalObject: object, "main": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreStackInst %7: any, %4: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %4: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [foo: any, capture_me: any]

// CHECK:function main(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.foo]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.capture_me]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS1.foo]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any]

// CHECK:function foo(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.x]: any
// CHECK-NEXT:  %4 = ResolveScopeInst (:environment) %VS1: any, %VS2: any, %1: environment
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %4: environment, [%VS1.capture_me]: any
// CHECK-NEXT:       ReturnInst %5: any
// CHECK-NEXT:function_end
