/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -target=HBC -dump-ir -O %s | %FileCheckOrRegen --match-full-lines %s

function thisUndefined () {
    'use strict';
    function inner() {
        return this;
    }
    return inner();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "thisUndefined": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %thisUndefined(): undefined
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "thisUndefined": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function thisUndefined(): undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
