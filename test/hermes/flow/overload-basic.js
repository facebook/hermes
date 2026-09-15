/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Basic overload resolution by argument type.
class Foo {
  @Hermes.final @Hermes.overload
  bar(x: number): number { return x * 2; }
  @Hermes.final @Hermes.overload
  bar(x: string): string { return x + x; }

  test(): void {
    print(this.bar(21));
    print(this.bar("ab"));
  }
}

print("type-based");
// CHECK-LABEL: type-based
let f = new Foo();
f.test();
// CHECK-NEXT: 42
// CHECK-NEXT: abab

// Overload resolution from outside the class.
print("external");
// CHECK-LABEL: external
print(f.bar(10));
// CHECK-NEXT: 20
print(f.bar("cd"));
// CHECK-NEXT: cdcd

// Overload resolution by arity.
class Multi {
  @Hermes.final @Hermes.overload
  calc(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  calc(x: number, y: number): number { return x + y; }
  @Hermes.final @Hermes.overload
  calc(x: number, y: number, z: number): number { return x + y + z; }
}

print("arity");
// CHECK-LABEL: arity
let m = new Multi();
print(m.calc(1));
// CHECK-NEXT: 1
print(m.calc(1, 2));
// CHECK-NEXT: 3
print(m.calc(1, 2, 3));
// CHECK-NEXT: 6
