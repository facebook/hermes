/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -O0 -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -O -exec %s | %FileCheck --match-full-lines %s

'use strict';

// Ordinary static access deliberately exercises emitMemberLoad, not the
// dedicated super-read helper. Accessors use explicit class-name storage.
class StaticOrdering {
  static backing: number = 0;
  static getterCalls: number = 0;
  static setterCalls: number = 0;

  @Hermes.final
  static set value(v: number) {
    StaticOrdering.setterCalls++;
    StaticOrdering.backing = v;
  }

  @Hermes.final
  static get value(): number {
    StaticOrdering.getterCalls++;
    return StaticOrdering.backing;
  }
}

print(StaticOrdering.value);
print(StaticOrdering.getterCalls, StaticOrdering.setterCalls);
// CHECK-NOT: {{.+}}
// CHECK: 0
// CHECK-NEXT: 1 0

// Also cover ordinary static setters after moving their IRGen handling.
StaticOrdering.value = 9;
print(StaticOrdering.getterCalls, StaticOrdering.setterCalls);
print(StaticOrdering.value);
print(StaticOrdering.getterCalls, StaticOrdering.setterCalls);
// CHECK-NEXT: 1 1
// CHECK-NEXT: 9
// CHECK-NEXT: 2 1

class GetterFirst {
  static backing: number = 3;
  static getterCalls: number = 0;
  static setterCalls: number = 0;

  @Hermes.final
  static get value(): number {
    GetterFirst.getterCalls++;
    return GetterFirst.backing;
  }

  @Hermes.final
  static set value(v: number) {
    GetterFirst.setterCalls++;
    GetterFirst.backing = v;
  }
}

print(GetterFirst.value);
print(GetterFirst.getterCalls, GetterFirst.setterCalls);
GetterFirst.value = 11;
print(GetterFirst.getterCalls, GetterFirst.setterCalls);
print(GetterFirst.value);
print(GetterFirst.getterCalls, GetterFirst.setterCalls);
// CHECK-NEXT: 3
// CHECK-NEXT: 1 0
// CHECK-NEXT: 1 1
// CHECK-NEXT: 11
// CHECK-NEXT: 2 1
// CHECK-NOT: {{.+}}

