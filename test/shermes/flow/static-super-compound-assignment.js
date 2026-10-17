/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Intended location: test/shermes/flow/static-super-compound-assignment.js
// RUN: %shermes -typed -O0 -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -O -exec %s | %FileCheck --match-full-lines %s

'use strict';

class AccessorBase {
  static backing: number = 0;
  @Hermes.final
  static set value(v: number) {
    print(this === AccessorChild, v);
    AccessorBase.backing = v;
  }

  @Hermes.final
  static get value(): number {
    print(this === AccessorChild);
    return AccessorBase.backing;
  }

  @Hermes.final
  static set only(v: number) {
    print(this === AccessorChild, v);
    AccessorBase.backing = v;
  }
}

function rhs(): number {
  print(2);
  return 2;
}

// Lookup must find the accessors on the grandparent.
class AccessorMiddle extends AccessorBase {}

class AccessorChild extends AccessorMiddle {
  static write(): void {
    const assigned: number = (super.value = 5);
    print(assigned);

    const updated: number = (super.value += rhs());
    print(updated);
  }

  static writeOnly(): void {
    // Plain assignment must not attempt to read this setter-only property.
    print(super.only = 11);
  }

  static writeArrow(): void {
    const write = (): void => { super.value = 13; };
    write();
  }

  @Hermes.final
  static writeGeneric<T>(unused: T): void {
    super.value = 17;
  }

  static update(): void {
    // Each update must call the getter and setter exactly once, and return
    // the old or new value as appropriate.
    print(super.value++);
    print(++super.value);
  }
}

AccessorChild.write();
print(AccessorBase.backing);
// No getter call during plain assignment. Compound order: getter, RHS, setter.
// CHECK-NOT: {{.+}}
// CHECK: true 5
// CHECK-NEXT: 5
// CHECK-NEXT: true
// CHECK-NEXT: 2
// CHECK-NEXT: true 7
// CHECK-NEXT: 7
// CHECK-NEXT: 7

AccessorChild.writeOnly();
// CHECK-NEXT: true 11
// CHECK-NEXT: 11

AccessorChild.writeArrow();
// CHECK-NEXT: true 13

// Force compilation of the generic method specialization.
AccessorChild.writeGeneric<number>(0);
// CHECK-NEXT: true 17

AccessorChild.update();
// CHECK-NEXT: true
// CHECK-NEXT: true 18
// CHECK-NEXT: 17
// CHECK-NEXT: true
// CHECK-NEXT: true 19
// CHECK-NEXT: 19
print(AccessorBase.backing);
// CHECK-NEXT: 19
// CHECK-NOT: {{.+}}
