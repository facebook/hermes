/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -fno-std-globals %s 2>&1 ) | %FileCheckOrRegen %s --match-full-lines

class A {
  #x: number;
}

var a: A = new A();
a.#x;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}class-private-props-error.js:15:3: error: the private name "#x" was not declared in any enclosing class
// CHECK-NEXT:a.#x;
// CHECK-NEXT:  ^~
// CHECK-NEXT:Emitted 1 errors. exiting.
