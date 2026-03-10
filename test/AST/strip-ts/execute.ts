/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes --transform-ts %s | %FileCheck %s

type MyType = string | number;
interface Greeter { greet(): void; }

function identity<T>(val: T): T {
  return val;
}

let x: number = identity(42) as number;
print(x);
// CHECK: 42

class Foo<T> {
  value: T;
  constructor(v: T) {
    this.value = v;
  }
  get(): T {
    return this.value;
  }
}

let foo = new Foo<string>("hello");
print(foo.get());
// CHECK-NEXT: hello
