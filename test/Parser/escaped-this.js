/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function f1(v1 = this, \u0074his) {
  return [v1, this];
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}escaped-this.js:10:24: warning: scanning identifier with unicode escape as reserved word
// CHECK-NEXT:function f1(v1 = this, \u0074his) {
// CHECK-NEXT:                       ^~~~~~~~~
// CHECK-NEXT:{{.*}}escaped-this.js:10:24: error: identifier, '{' or '[' expected in binding pattern
// CHECK-NEXT:function f1(v1 = this, \u0074his) {
// CHECK-NEXT:                       ^
// CHECK-NEXT:Emitted 1 errors. exiting.
