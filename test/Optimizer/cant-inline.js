/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheck --match-full-lines %s


// Make sure that we are not inlining if copyRestArgs() is used.

function outer1() {
    return (function dontInline(...rest) {
        return rest;
    })(1);
}
//CHECK-LABEL:function outer1()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CreateFunctionInst %dontInline()
//CHECK-NEXT:  %1 = CallInst %0 : closure, undefined : undefined, 1 : number
//CHECK-NEXT:  %2 = ReturnInst %1
//CHECK-NEXT:function_end

//CHECK-LABEL:function dontInline()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = CallBuiltinInst [HermesBuiltin.copyRestArgs] : number, undefined : undefined, 0 : number
//CHECK-NEXT:  %1 = ReturnInst %0
//CHECK-NEXT:function_end
