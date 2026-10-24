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

// A class with a static SH_call method may be invoked without 'new'; the plain
// call dispatches to SH_call and returns its declared return type. 'new' still
// runs the constructor.
class Sym {
  desc: string;
  constructor(desc: string) {
    this.desc = desc;
  }
  static SH_call(desc: string): string {
    return 'called:' + desc;
  }
}

// Without 'new' -> SH_call.
print(Sym('a'));
// CHECK-LABEL: called:a

// With 'new' -> constructor.
print(new Sym('b').desc);
// CHECK-NEXT: b

// The result type of a plain call is SH_call's return type.
var s: string = Sym('c');
print(s.length);
// CHECK-NEXT: 8

// SH_call can return any type, unrelated to the class.
class Answer {
  static SH_call(): number {
    return 42;
  }
}
var n: number = Answer();
print(n);
// CHECK-NEXT: 42

// SH_call receives its arguments and 'this' is the class constructor.
class Adder {
  static base: number = 100;
  static SH_call(a: number, b: number): number {
    return Adder.base + a + b;
  }
}
print(Adder(2, 3));
// CHECK-NEXT: 105
