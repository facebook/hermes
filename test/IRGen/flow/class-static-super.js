/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

class Base {
  static value: number = 3;
  static backing: number = 3;
  static label: string = "base";

  static method(): number {
    return Base.value + 4;
  }

  @Hermes.final static get current(): number {
    return this.backing;
  }

  @Hermes.final static set current(value: number) {
    this.backing = value;
  }

  @Hermes.final static identify<T>(value: T): string {
    return this.label;
  }
}

class Child extends Base {
  static label: string = "child";

  static testMethod(): number {
    return super.method();
  }

  static testAssignment(): number {
    super.value = 9;
    return Base.value;
  }

  static testAccessors(): number {
    super.current = 42;
    return super.current;
  }

  static testGenericThis(): string {
    return super.identify<number>(1);
  }
}

class ChildA extends Base {
  static label: string = "a";

  static testGenericThis(): string {
    return super.identify<number>(1);
  }
}

class ChildB extends Base {
  static label: string = "b";

  static testGenericThis(): string {
    return super.identify<number>(1);
  }
}

print(Child.testMethod());
print(Child.testAssignment());
print(Child.testAccessors());
print(Child.testGenericThis());
print(ChildA.testGenericThis());
print(ChildB.testGenericThis());

// CHECK: 7
// CHECK-NEXT: 9
// CHECK-NEXT: 42
// CHECK-NEXT: child
// CHECK-NEXT: a
// CHECK-NEXT: b
