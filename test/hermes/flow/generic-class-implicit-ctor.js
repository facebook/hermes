/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -typed %s | %FileCheck %s --match-full-lines
// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

class A<T> {
  foo(): void {
    print('foo');
  }
}

var a: A<number> = new A<number>();
a.foo();
// CHECK: foo
