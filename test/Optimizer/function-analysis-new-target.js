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
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %main(): object
// CHECK-NEXT:       StorePropertyStrictInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst "use strict": string
// CHECK-NEXT:function_end

// CHECK:function main(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) %f(): undefined
// CHECK-NEXT:  %1 = LoadPropertyInst (:any) %0: object, "prototype": string
// CHECK-NEXT:  %2 = CreateThisInst (:object) %1: any, %0: object
// CHECK-NEXT:  %3 = CallInst (:undefined) %0: object, %f(): undefined, empty: any, %0: object, %2: object
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:function f(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %1 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:  %2 = CallInst (:any) %0: any, empty: any, empty: any, undefined: undefined, undefined: undefined, %1: undefined|object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
