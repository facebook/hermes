/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Overloaded methods are inherited by subclasses.
class Base {
  @Hermes.final @Hermes.overload
  foo(x: number): number { return x * 10; }
  @Hermes.final @Hermes.overload
  foo(x: string): string { return x + "!"; }
}

class Child extends Base {
  test(): void {
    print(this.foo(5));
    print(this.foo("hi"));
  }
}

print("inherit");
// CHECK-LABEL: inherit
let c = new Child();
c.test();
// CHECK-NEXT: 50
// CHECK-NEXT: hi!

// Also callable from outside the class on the subclass instance.
print(c.foo(3));
// CHECK-NEXT: 30
print(c.foo("ok"));
// CHECK-NEXT: ok!
