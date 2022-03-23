/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

class Foo {
  get x<T>() { }
}
// CHECK: {{.*}}:11:3: error: accessor method may not have type parameters
// CHECK:   get x<T>() { }
// CHECK:   ^~~~~~~~~~~~~~
