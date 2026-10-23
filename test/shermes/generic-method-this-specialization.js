/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -O0 -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -O -exec %s | %FileCheck --match-full-lines %s

'use strict';

class ReceiverBase {
  id: number = 10;

  @Hermes.final
  baseFirst<T>(value: T): number {
    return this.id;
  }

  @Hermes.final
  derivedFirst<T>(value: T): T {
    return this.id;
  }
}

class ReceiverDerived extends ReceiverBase {
  id: number = 20;
}
const derived = new ReceiverDerived();
const base = new ReceiverBase();
print(base.baseFirst < number > (1));
//CHECK: 10
print(derived.baseFirst < number > (1));
//CHECK-NEXT: 20
print(derived.derivedFirst < number > (1));
//CHECK-NEXT: 20
print(base.derivedFirst < number > (1));
//CHECK-NEXT: 10


class StaticReceiverBase {
  static id: number = 10;

  @Hermes.final
  static baseFirst<T>(value: T): number {
    return StaticReceiverBase.id;
  }

  @Hermes.final
  static derivedFirst<T>(value: T): number {
    return StaticReceiverBase.id;
  }
}

class StaticReceiverDerived extends StaticReceiverBase {
}
print(StaticReceiverBase.baseFirst < number > (1));
//CHECK-NEXT: 10
print(StaticReceiverDerived.baseFirst < number > (1));
//CHECK-NEXT: 10
print(StaticReceiverDerived.derivedFirst < number > (1));
//CHECK-NEXT: 10
print(StaticReceiverBase.derivedFirst < number > (1));
//CHECK-NEXT: 10
