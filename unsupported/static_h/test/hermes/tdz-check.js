/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xenable-tdz -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xenable-tdz -O %s | %FileCheck --match-full-lines %s

function test(f) {
    try {
        f();
    } catch (e) {
        print("caught", e);
    }
}

print("begin");
//CHECK: begin

test(() => {
    print(a);
    let a = 10;
});
//CHECK-NEXT: caught ReferenceError: accessing an uninitialized variable

test(() => {
    a = 10;
    let a;
});
//CHECK-NEXT: caught ReferenceError: accessing an uninitialized variable

test((p1) => {
    if (p1)
        print(a);
    let a;
    print("OK", a);
});
//CHECK-NEXT: OK undefined

test(() => {
    let x = init();
    function init(p1) { return p1 ? 10 : x; }
    return x;
});
//CHECK-NEXT: caught ReferenceError: accessing an uninitialized variable

test(() => {
    const foo = print(foo, typeof foo)
});
//CHECK-NEXT: caught ReferenceError: accessing an uninitialized variable
