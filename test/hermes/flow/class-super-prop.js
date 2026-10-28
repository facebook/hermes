/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Werror -typed -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O %s | %FileCheck --match-full-lines %s

'use strict';

class A {
  x: number;

  constructor(x: number) {
    this.x = x;
  }
}

class B extends A {
  y: number;

  constructor(x: number, y: number) {
    super(x);
    this.y = y;
  }

  f(): number {
    return super.x + this.x + this.y;
  }
}

print('super prop');
// CHECK-LABEL: super prop
print(new A(2).x);
// CHECK-NEXT: 2
print(new B(3, 10).f());
// CHECK-NEXT: 16

// A field whose type has no representable default is stored uninitialized, so
// reading it through 'super' must IDZ-check exactly like reading it through
// 'this'.
class E {
  n: number = 1;
}

class P {
  f: E;
}

class C extends P {
  viaSuper(): E {
    return super.f;
  }
  viaThis(): E {
    return this.f;
  }
}

const c = new C();
try {
  c.viaThis();
} catch (e) {
  print('this:', e.name);
}
// CHECK-NEXT: this: ReferenceError
try {
  c.viaSuper();
} catch (e) {
  print('super:', e.name);
}
// CHECK-NEXT: super: ReferenceError
c.f = new E();
print(c.viaSuper().n, c.viaSuper() === c.viaThis());
// CHECK-NEXT: 1 true
