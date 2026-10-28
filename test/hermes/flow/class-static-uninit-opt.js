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

// A static with no representable default stays unset until written, and
// reading it first must throw. The optimizer must not treat the default store
// at the top of the scope as an initialization and drop the check.
class E {
  x: number = 3;
}

class A {
  static o: E;
  static set(): void {
    A.o = new E();
  }
}

try {
  print(A.o.x);
} catch (e) {
  print(e.name);
}
// CHECK: ReferenceError
A.set();
print(A.o.x);
// CHECK-NEXT: 3

// The same through a forward reference in a static initializer, with the
// class kept alive by an escaping closure.
let escaped: any[] = [];
function defineF(): void {
  class F {
    static early: E = F.late;
    static late: E = new E();
    static keep(): E {
      return F.late;
    }
  }
  escaped.push(F.keep);
}

try {
  defineF();
  print('no throw');
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: ReferenceError
