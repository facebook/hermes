/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue -O -gc-sanitize-handles=1 -target=HBC %s | %FileCheck --match-full-lines %s

"use strict";

print('weakref with Symbol target');
// CHECK-LABEL: weakref with Symbol target

try { let ref = new WeakRef(Symbol.for("test")); } catch(e) { print("caught", e.name, e.message); }
// CHECK-NEXT: caught TypeError target argument is not an object or non-registered symbol

globalThis.objs = [];
var refs = [];
var n = 1000;
for (let i = 0; i < n; ++i) {
    objs[i] = Symbol(i);
    refs[i] = new WeakRef(objs[i]);
}

function testDerefSymbol() {
    var acc = 0;
    for (var i = 0; i < n; ++i) {
        acc += Number(refs[i].deref().description);
    }
    print(acc);
    // CHECK: 499500
}

setTimeout(testDerefSymbol, 0);
