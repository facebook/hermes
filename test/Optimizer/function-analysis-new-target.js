/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -fno-inline -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict';

// Function leaks by usage of new.target.
function main() {
  var x;
  x = function f() {
    sink(new.target);
  };
  return new x();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): string
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %main(): functionCode
// CHECK-NEXT:       StorePropertyStrictInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %f(): functionCode
// CHECK-NEXT:  %1 = CreateThisInst (:object) %0: object, %0: object
// CHECK-NEXT:  %2 = CallInst (:undefined) %0: object, %f(): functionCode, true: boolean, empty: any, %0: object, %1: object
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function f(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %1 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %1: undefined|object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
