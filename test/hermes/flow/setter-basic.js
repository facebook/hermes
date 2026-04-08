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

// Test instance getter+setter pair.
class Counter {
  _count: number;

  constructor() {
    this._count = 0;
  }

  @Hermes.final
  get count(): number {
    return this._count;
  }

  @Hermes.final
  set count(v: number): void {
    this._count = v;
  }
}

let c = new Counter();
print(c.count);
// CHECK: 0
c.count = 42;
print(c.count);
// CHECK-NEXT: 42

// Test static getter+setter pair.
class Config {
  static _debug: boolean = true;

  @Hermes.final
  static get debug(): boolean {
    return Config._debug;
  }

  @Hermes.final
  static set debug(v: boolean): void {
    Config._debug = v;
  }
}

print(Config.debug);
// CHECK-NEXT: true
Config.debug = false;
print(Config.debug);
// CHECK-NEXT: false

// Test setter-only (no getter).
class WriteOnly {
  _val: number;

  constructor() {
    this._val = 0;
  }

  @Hermes.final
  set val(v: number): void {
    this._val = v;
  }
}

let w = new WriteOnly();
w.val = 99;
print(w._val);
// CHECK-NEXT: 99

// Test setter in expression (compound not allowed for getter-only,
// but simple assignment works).
c.count = c.count + 5;
print(c.count);
// CHECK-NEXT: 47
