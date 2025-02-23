/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function foo() {
  var undefined = 5;
  return undefined;
}

var undefined = 5;
foo();
undefined;

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:       DeclareGlobalVarInst "undefined": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "foo": string
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %5: any
// CHECK-NEXT:       StorePropertyLooseInst 5: number, globalObject: object, "undefined": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) globalObject: object, "foo": string
// CHECK-NEXT:  %9 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        StoreStackInst %9: any, %5: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %5: any
// CHECK-NEXT:  %12 = LoadStackInst (:any) %5: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [undefined: any]

// CHECK:function foo(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.undefined]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 5: number, [%VS1.undefined]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS1.undefined]: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end
