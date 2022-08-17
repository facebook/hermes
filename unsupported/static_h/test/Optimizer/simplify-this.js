/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ir -O %s | %FileCheck --match-full-lines %s

function thisUndefined () {
    function inner() {
        return this;
    }
    return inner();
}

//CHECK-LABEL:function thisUndefined() : object
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst globalObject : object
//CHECK-NEXT:function_end
