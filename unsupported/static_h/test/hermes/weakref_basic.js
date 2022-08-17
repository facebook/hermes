/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue -O -gc-sanitize-handles=1 -target=HBC %s | %FileCheck --match-full-lines %s

"use strict";

print('weakref');
// CHECK: weakref

globalThis.objs = [];
var refs = [];
var n = 1000;
for (var i = 0; i < n; ++i) {
  objs[i] = {foo: i};
  refs[i] = new WeakRef(objs[i]);
}

function testDeref(){
  var acc = 0;
  for (var i = 0; i < n; ++i) {
    acc += refs[i].deref().foo;
  }
  print(acc);
  // CHECK: 499500
}

setTimeout(testDeref, 0);
