/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb --lazy %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function foo() {
  print('foo called');
}

function bar() {
  print('bar called');
}

print('hello');
// CHECK: hello
foo();
// CHECK: foo called
