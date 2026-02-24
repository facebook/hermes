/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -O0 -g2 %s | %FileCheck --match-full-lines %s

// Check that we are able to successfully create a heap snapshot in the presence
// of synthetic functions generated from class (which may not have source
// location information).

print('Create heap snapshot');
// CHECK: Create heap snapshot

class A {
  static f1 = 0;
  static f2;

  static #p1;
  static #p2 = 42;

  #p3;
  #p4 = {};

  static {
    this.f2 = {init: true};
    this.#p1 = {private: true};
  }

  constructor(b) {
    this.#p3 = b;
  }
}

const a = new A({});
try {
  createHeapSnapshot('a.heapsnapshot');
} catch (e) {
  // Catch the exception when memory instrumentation is not enabled and
  // createHeapSnapshot throws TypeError.
}

print('Done');
// CHECK: Done
