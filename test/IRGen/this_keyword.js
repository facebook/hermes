/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function f1() {
  return this;
}

function f2(){
  "use strict"; // see strict mode
  return this;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "f1": string
// CHECK-NEXT:       DeclareGlobalVarInst "f2": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %f1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "f1": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %f2(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "f2": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f1(): object
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:       ReturnInst %1: object
// CHECK-NEXT:function_end

// CHECK:function f2(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:       ReturnInst %0: any
// CHECK-NEXT:function_end
