/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Function leaks as an arg to its own call.
function main() {
  var x;
  x = function f() {
    sink();
  };
  return x(x);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %main(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %2: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %f(): functionCode
// CHECK-NEXT:  %2 = CallInst (:undefined) %1: object, %f(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined, %1: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %1 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
