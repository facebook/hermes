/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s -dump-sema -fno-std-globals 2>&1) | %FileCheck --match-full-lines %s

class C extends ( class { x = this.#foo; } ) {
  #foo;
};

// CHECK:{{.*}}private-name-in-extends-error.js:10:36: error: the private name "#foo" was not declared in any enclosing class
// CHECK-NEXT:class C extends ( class { x = this.#foo; } ) {
// CHECK-NEXT:                                   ^~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
