/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

// Ensure all the dead functions inside main get cleaned up correctly,
// without leaving dangling load/store to uncreated VariableScopes.
function main() {
  function foo() {
    function resume() {
      sink(function () {
        resume();
      });
    }
    function settle() {
      resume();
    }
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "main": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %main(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "main": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function main(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
