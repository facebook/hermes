/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

function get<T>(x: T): A<T> {
  return new A<T>(x);
}
class A<T> {
  x: T;
  constructor(x: T) {
    this.x = x;
  }
  bar(): string {
    return 'bar';
  }
}
class B {
  foo(): void {
    const text = get<string>('foo');
    print(text.x);
    const other = text.bar();
    print(other);
  }
}

print('generic class');
// CHECK-LABEL: generic class

let val = get<string>('abc');
print(val.x);
// CHECK-NEXT: abc
new B().foo();
// CHECK-NEXT: foo
// CHECK-NEXT: bar
