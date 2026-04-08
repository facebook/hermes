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

class Counter {
  _count: number;

  constructor() {
    this._count = 0;
  }

  @Hermes.final
  get count(): number {
    return this._count;
  }

  increment(): void {
    this._count += 1;
  }
}

let c = new Counter();
print(c.count);
// CHECK: 0
c.increment();
print(c.count);
// CHECK-NEXT: 1
c.increment();
c.increment();
print(c.count);
// CHECK-NEXT: 3

// Test typeof on getter result.
print(typeof c.count);
// CHECK-NEXT: number

// Test getter in expression.
print(c.count + 10);
// CHECK-NEXT: 13

// Test static getter.
class Config {
  static _debug: boolean = true;

  @Hermes.final
  static get debug(): boolean {
    return Config._debug;
  }
}

print(Config.debug);
// CHECK-NEXT: true
Config._debug = false;
print(Config.debug);
// CHECK-NEXT: false
