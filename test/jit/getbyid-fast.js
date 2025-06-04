/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -stats -fno-inline -Xjit -Xjit-crash-on-error %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: jit, debug_options

// Test that exactly two specializations are emitted: one direct, one for the parent.

var o = {p: 10};

function foo(o) {
    return o.call;
}
function bar(o) {
    return o.p;
}

let call = Function.prototype.call;

for(let i = 0; i < 100; ++i) {
    let tmp;
    tmp = foo(foo);
    if (tmp != call) throw Error("call not returned");
    tmp = bar(o);
    if (tmp != 10) throw Error("10 not returned");
}

// CHECK: 2 jit                  - JITNumGetByIdSpec: number of GetById specialized fast paths emitted
