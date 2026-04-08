/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -typed %s | %FileCheck %s --match-full-lines
// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -O0 -typed -exec %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Implicit derived constructor forwards args to super.
class Base {
  x: number;
  constructor(x: number) { this.x = x; }
}
class Derived extends Base {
}
var d = new Derived(42);
print(d.x);
// CHECK: 42

// Derived class with own fields inherits super's constructor.
class Derived2 extends Base {
  y: number = 99;
}
var d2 = new Derived2(7);
print(d2.x);
// CHECK: 7
print(d2.y);
// CHECK: 99

// Chain of implicit constructors.
class Mid extends Derived2 {
}
var m = new Mid(3);
print(m.x);
// CHECK: 3
print(m.y);
// CHECK: 99

// Implicit constructor with multiple params.
class Multi {
  a: number;
  b: string;
  constructor(a: number, b: string) {
    this.a = a;
    this.b = b;
  }
}
class MultiChild extends Multi {
}
var mc = new MultiChild(123, "hello");
print(mc.a);
// CHECK: 123
print(mc.b);
// CHECK: hello

// Implicit constructor with no-arg base.
class NoArgs {
  z: number = 55;
}
class NoArgsChild extends NoArgs {
}
var na = new NoArgsChild();
print(na.z);
// CHECK: 55
