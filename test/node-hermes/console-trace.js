/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: node-hermes

function foo() {
  function bar() {
    console.trace("This is a trace message");
  }
  bar();
}

foo();
// CHECK: Trace: This is a trace message
// CHECK-NEXT:    at bar ({{.*}})
// CHECK-NEXT:    at foo ({{.*}})
// CHECK-NEXT:    at global ({{.*}})
