/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

let outer: A<number> | null = null;

class B<T> {
  constructor() {}
  read(): void {
    (globalThis.a as A<boolean>).foo();
  }
}

class A<T> extends B<T> {
  constructor() {
    super();
  }
  foo(): void {
    print('foo')
  }
}

globalThis.a = new A<boolean>();
new B<number>().read();
// CHECK: foo
