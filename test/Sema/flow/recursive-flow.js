/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-sema %s

type A = {readonly next: A | null};
type B = {readonly next: B | null};

function bounded<T extends A>(value: T): T {
  return value;
}

function checkBounded(value: B): void {
  bounded(value);
}

type WriteA = {-next: WriteA | null};
type WriteB = {-next: WriteB | null};

function convertWrite(value: WriteA): WriteB {
  return value;
}

class Base {
  method(value: A): A {
    return value;
  }
}

class Derived extends Base {
  method(value: B): B {
    return value;
  }
}
