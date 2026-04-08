/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed -O0 %s | %FileCheck %s --match-full-lines
// RUN: %hermes -typed -O %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

'use strict';

// Private getter only.
class Counter {
  _count: number;

  constructor() {
    this._count = 0;
  }

  @Hermes.final
  get #count(): number {
    return this._count;
  }

  increment(): void {
    this._count += 1;
  }

  getCount(): number {
    return this.#count;
  }
}

let c = new Counter();
print(c.getCount());
// CHECK: 0
c.increment();
c.increment();
print(c.getCount());
// CHECK-NEXT: 2

// Private getter + setter pair.
class Box {
  _val: number;

  constructor(v: number) {
    this._val = v;
  }

  @Hermes.final
  get #val(): number {
    return this._val;
  }

  @Hermes.final
  set #val(v: number): void {
    this._val = v;
  }

  getVal(): number {
    return this.#val;
  }

  setVal(v: number): void {
    this.#val = v;
  }
}

let b = new Box(10);
print(b.getVal());
// CHECK-NEXT: 10
b.setVal(42);
print(b.getVal());
// CHECK-NEXT: 42

// Private setter only.
class Logger {
  _last: number;

  constructor() {
    this._last = 0;
  }

  @Hermes.final
  set #last(v: number): void {
    this._last = v;
  }

  record(v: number): void {
    this.#last = v;
  }
}

let log = new Logger();
log.record(99);
print(log._last);
// CHECK-NEXT: 99
