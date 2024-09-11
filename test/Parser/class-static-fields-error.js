/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc %s -dump-ast 2>&1 ) | %FileCheck --match-full-lines %s

class A {
  static prototype = 4;
}
// CHECK: {{.*}}class-static-fields-error.js:11:10: error: Static class properties cannot be named 'prototype'

class A {
  static "prototype" = 4;
}
// CHECK: {{.*}}class-static-fields-error.js:16:10: error: Static class properties cannot be named 'prototype'
