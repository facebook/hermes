/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -gc-sanitize-handles=0 %s | %FileCheck %s
// Check that long stack traces are truncated correctly at 100 entries.
"use strict";

var funcs = [];
funcs[0] = function foo0(a) {
    throw Error("deep stack");
}
for(var i = 1; i < 150; ++i) {
    funcs.push(eval("(function foo"+i+"(a) { return funcs["+(i-1)+"](a+1); })"));
}

try {
    funcs[i-1](0);
} catch (e) {
    print("caught exception");
    print(e.stack);
    print("end");
}
//CHECK:      caught exception
//CHECK-NEXT: {{.*}}deep stack
//CHECK-NEXT: at foo0
//CHECK-NEXT: at foo1
//CHECK:      ... skipping 51 frames
//CHECK-NEXT: at foo101
//CHECK-NEXT: at foo102
//CHECK:      at foo148
//CHECK-NEXT: at foo149
//CHECK-NEXT: at global
//CHECK-NEXT: end
