/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xmicrotask-queue -O %s | %FileCheck --match-full-lines %s

"use strict";

print("parent transition retention");
// CHECK-LABEL: parent transition retention

var N = 100;
var cyclic = [];
var statics = [];
var plain = [];

// A prototype that references one of its own instances.
function makeCyclic() {
  var proto = {};
  proto.inst = Object.create(proto);
  return new WeakRef(proto);
}

// A class whose static field holds an instance.
function makeClass() {
  class C {}
  C.instance = new C();
  return new WeakRef(C);
}

// A prototype with no path back to its instances.
function makePlain() {
  var proto = {};
  Object.create(proto);
  return new WeakRef(proto);
}

for (var i = 0; i < N; ++i) {
  cyclic.push(makeCyclic());
  statics.push(makeClass());
  plain.push(makePlain());
}

function alive(refs) {
  var n = 0;
  for (var i = 0; i < refs.length; ++i)
    if (refs[i].deref() !== undefined) ++n;
  return n;
}

// WeakRef targets are kept alive until the end of the current job, so
// collect and check in later tasks.
setTimeout(function () {
  gc();
  setTimeout(function () {
    print(
      "cyclic " + alive(cyclic) + ", class " + alive(statics) +
        ", plain " + alive(plain),
    );
  }, 0);
}, 0);
// CHECK-NEXT: cyclic 0, class 0, plain 0
