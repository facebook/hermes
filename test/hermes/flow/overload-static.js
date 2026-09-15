/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Static overload resolution by argument type.
class Foo {
  @Hermes.final @Hermes.overload
  static bar(x: number): number { return x * 2; }
  @Hermes.final @Hermes.overload
  static bar(x: string): string { return x + x; }

  // Call static overloaded method from inside the class.
  static test(): void {
    print(Foo.bar(21));
    print(Foo.bar("ab"));
  }
}

print("type-based");
// CHECK-LABEL: type-based
Foo.bar(21);
print(Foo.bar(21));
// CHECK-NEXT: 42
print(Foo.bar("cd"));
// CHECK-NEXT: cdcd

// Call from inside the class.
print("internal");
// CHECK-LABEL: internal
Foo.test();
// CHECK-NEXT: 42
// CHECK-NEXT: abab

// Static overload resolution by arity.
class Multi {
  @Hermes.final @Hermes.overload
  static calc(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  static calc(x: number, y: number): number { return x + y; }
  @Hermes.final @Hermes.overload
  static calc(x: number, y: number, z: number): number { return x + y + z; }
}

print("arity");
// CHECK-LABEL: arity
print(Multi.calc(1));
// CHECK-NEXT: 1
print(Multi.calc(1, 2));
// CHECK-NEXT: 3
print(Multi.calc(1, 2, 3));
// CHECK-NEXT: 6
