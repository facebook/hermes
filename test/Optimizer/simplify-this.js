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

// CHECK:function global() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "thisUndefined" : string
// CHECK-NEXT:  %1 = CreateFunctionInst %thisUndefined() : undefined
// CHECK-NEXT:  %2 = StorePropertyLooseInst %1 : closure, globalObject : object, "thisUndefined" : string
// CHECK-NEXT:  %3 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function thisUndefined() : undefined
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end
