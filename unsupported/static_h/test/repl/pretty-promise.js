// Remove this once the REPL can handle block comments.
// @lint-ignore-every LICENSELINT

// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

// RUN: cat %s | %hermes -Xes6-promise -prompt="" -prompt2="" | %FileCheck --match-full-lines %s

"pretty Promise"
// CHECK-LABEL: "pretty Promise"

Promise.resolve(1)
// CHECK-NEXT: Promise <fulfilled: 1>
a = [1]; b = {a}; Promise.resolve(b)
// CHECK-NEXT: Promise <fulfilled: { a: [ 1, [length]: 1 ] }>

p = new Promise((res, rej) => setTimeout(_ => res(1), 1000));
// CHECK-NEXT: Promise <pending>
p
// CHECK-NEXT: Promise <fulfilled: 1>
p = new Promise((res, rej) => setTimeout(_ => rej(1), 1000));
// CHECK-NEXT: Promise <pending>
p
// CHECK-NEXT: Promise <rejected: 1>

p = Promise.resolve(1)
// CHECK-NEXT: Promise <fulfilled: 1>
p.foo = 1; p;
// CHECK-NEXT: Promise <fulfilled: 1> { foo: 1 }

"Promise adoption"
// CHECK-LABEL: "Promise adoption"

new Promise((res) => { res(Promise.resolve(1)) })
// CHECK-NEXT: Promise <fulfilled: 1>

new Promise((res) => {
    res(
        new Promise((res) => { res(Promise.resolve(1)) })
    )
})
// CHECK-NEXT: Promise <fulfilled: 1>

new Promise((res, rej) => { rej(Promise.resolve(1)) })
// CHECK-NEXT: Promise <rejected: Promise <fulfilled: 1>>

new Promise((res, rej) => {
    rej(
        new Promise((res) => { res(Promise.resolve(1)) })
    )
})
// CHECK-NEXT: Promise <rejected: Promise <fulfilled: 1>>


"User-space Promise"
// CHECK-LABEL: "User-space Promise"

// User space Promise can override w/o problems and not break ours
// NO Hermes Promise testing should go below here.

p = Promise.resolve(1)
// CHECK-NEXT: Promise <fulfilled: 1>
function Promise() { this.foo = 'foo' }; new Promise();
// CHECK-NEXT: Promise { foo: "foo" }
p;
// CHECK-NEXT: Promise <fulfilled: 1>


