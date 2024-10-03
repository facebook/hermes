/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Resolve construction through a variable.
function main() {
  var x;
  x = function f() {};
  return new x();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %main(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %2: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %f(): functionCode
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: object, empty: any
// CHECK-NEXT:  %3 = CallInst (:undefined) %1: object, %f(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function f(): undefined [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
