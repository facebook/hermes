/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

// IRGen emits a finally body once per control-flow exit path. Each `try` below
// contains a `return` so that the copy for the return path is emitted first;
// the paths actually taken here are therefore the later copies, which used to
// be missing the initialization of the async function's hidden inner closure.

print("async function in finally");
// CHECK-LABEL: async function in finally

// Normal completion path.
function declNormal(cond) {
  try {
    if (cond) return;
  } finally {
    async function g() { return "decl-normal"; }
    g().then(print);
  }
}
declNormal(false);

// Exception (rethrow) path.
function declThrow() {
  try {
    if (arguments[0]) return;
    throw new Error("boom");
  } finally {
    async function g() { return "decl-throw"; }
    g().then(print);
  }
}
try { declThrow(false); } catch (e) {}

// Async arrow capturing 'this' and 'arguments', normal completion path.
function arrowNormal(cond) {
  try {
    if (cond) return;
  } finally {
    var g = async () => "arrow-normal " + this.tag + " " + arguments[1];
    g().then(print);
  }
}
arrowNormal.call({tag: "T"}, false, 7);

// CHECK-NEXT: decl-normal
// CHECK-NEXT: decl-throw
// CHECK-NEXT: arrow-normal T 7
