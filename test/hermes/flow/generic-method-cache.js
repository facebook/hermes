/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

class Wrapper {
  tag: string;

  constructor() {
    this.tag = "wrap";
  }

  @Hermes.final
  echo<T>(val: T): T {
    return val;
  }
}

let w: Wrapper = new Wrapper();

// Multiple call sites with the same specialization (number).
// Exercises the specialization cache hit path.
print(w.echo<number>(1));
// CHECK: 1
print(w.echo<number>(2));
// CHECK-NEXT: 2
print(w.echo<number>(3));
// CHECK-NEXT: 3

// Multiple call sites with the same specialization (string).
print(w.echo<string>("a"));
// CHECK-NEXT: a
print(w.echo<string>("b"));
// CHECK-NEXT: b

// Inferred duplicates should also hit the cache.
print(w.echo(4));
// CHECK-NEXT: 4
print(w.echo(5));
// CHECK-NEXT: 5
