/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -target=HBC -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function foo(f) {
  // Must avoid the applyArguments optimization here.
  arguments[0] = 12;
  f.apply(this, arguments);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(f: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateArgumentsLooseInst (:object)
// CHECK-NEXT:  %1 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %2 = CoerceThisNSInst (:object) %1: any
// CHECK-NEXT:  %3 = LoadParamInst (:any) %f: any
// CHECK-NEXT:       StorePropertyLooseInst 12: number, %0: object, 0: number
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %3: any, "apply": string
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, false: boolean, empty: any, undefined: undefined, %3: any, %2: object, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
