/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

var o = {
  async
  foo() {}
};

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}async-method-newline-error.js:12:3: error: newline not allowed after 'async' in a method definition
// CHECK-NEXT:  foo() {}
// CHECK-NEXT:  ^~~
