/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheck --match-full-lines %s

function foo() {
    return eval("1 + 1");
}
//CHECK-LABEL:function foo()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = DirectEvalInst "1 + 1" : string
//CHECK-NEXT:  %1 = ReturnInst %0
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %2 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function bar() {
    return eval("2 + 2", Math, foo());
}
//CHECK-LABEL:function bar()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "Math" : string
//CHECK-NEXT:  %1 = LoadPropertyInst globalObject : object, "foo" : string
//CHECK-NEXT:  %2 = CallInst %1, undefined : undefined
//CHECK-NEXT:  %3 = DirectEvalInst "2 + 2" : string
//CHECK-NEXT:  %4 = ReturnInst %3
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %5 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function baz() {
    return eval();
}
//CHECK-LABEL:function baz()
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %1 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
