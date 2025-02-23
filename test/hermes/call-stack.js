/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Wx,-Xhermes-internal-test-methods -g -exec %s | %FileCheck --match-full-lines %s
"use strict";

function c() {
  return HermesInternal.getCallStack();
}

function b() {
  var f = function() {
    return c();
  };
  return f();
}

function a() {
  return b();
}

print(a());
//CHECK: c: {{.*/call-stack.js}}:13:37
//CHECK-NEXT: b: {{.*/call-stack.js}}:18:13
//CHECK-NEXT: a: {{.*/call-stack.js}}:24:11
//CHECK-NEXT: global: {{.*/call-stack.js}}:27:8
