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
  static y: number = 10;
  static #secret: number = 99;
  static foo(): number { return 42; }
  static getSecret(): number { return A.#secret; }

  constructor(x: number) {
    this.x = x;
  }

  bar(): number { return this.x; }
}

class B extends A {
  static z: string = "hello";
  static foo(): number { return 99; }

  constructor(x: number) {
    super(x);
  }
}

// Test static fields.
print("static fields");
// CHECK-LABEL: static fields
print(A.y);
// CHECK-NEXT: 10
print(B.z);
// CHECK-NEXT: hello

// Test static methods.
print("static methods");
// CHECK-LABEL: static methods
print(A.foo());
// CHECK-NEXT: 42
print(B.foo());
// CHECK-NEXT: 99

// Test private static fields.
print("private static");
// CHECK-LABEL: private static
print(A.getSecret());
// CHECK-NEXT: 99

// Test instance functionality still works with static members present.
print("instance");
// CHECK-LABEL: instance
print(new A(5).bar());
// CHECK-NEXT: 5
print(new B(7).bar());
// CHECK-NEXT: 7

// Test static private methods.
class C {
  static #secret: number = 42;
  static #getSecret(): number { return C.#secret; }
  static #double(x: number): number { return x * 2; }
  static callGetSecret(): number { return C.#getSecret(); }
  static callDouble(x: number): number { return C.#double(x); }
}

print("static private methods");
// CHECK-LABEL: static private methods
print(C.callGetSecret());
// CHECK-NEXT: 42
print(C.callDouble(21));
// CHECK-NEXT: 42

// Test writing to static fields.
print("static field write");
// CHECK-LABEL: static field write
A.y = 42;
print(A.y);
// CHECK-NEXT: 42

// Test increment/decrement of static fields.
print("static field increment");
// CHECK-LABEL: static field increment
A.y = 0;
A.y++;
A.y++;
A.y++;
print(A.y);
// CHECK-NEXT: 3

// Test static field as mutable counter (the widgets.js pattern).
class Counter {
  static id: number = 0;
  static next(): number {
    return Counter.id++;
  }
}
print("static counter");
// CHECK-LABEL: static counter
print(Counter.next());
// CHECK-NEXT: 0
print(Counter.next());
// CHECK-NEXT: 1
print(Counter.next());
// CHECK-NEXT: 2

// Test writing a static private field, in every form.
class W {
  static #n: number = 1;
  static bump(): void {
    W.#n = 5;
    W.#n += 10;
    W.#n++;
  }
  @Hermes.final static get val(): number {
    return W.#n;
  }
  @Hermes.final static set val(v: number) {
    W.#n = v;
  }
}
print("static private writes");
// CHECK-LABEL: static private writes
W.bump();
print(W.val);
// CHECK-NEXT: 16
W.val = 7;
print(W.val);
// CHECK-NEXT: 7

// Static fields are default-initialized before any initializer runs, so a
// reference to a field declared later reads the default, matching the way
// instance fields behave. The same holds through 'this' and private names.
class FR {
  static a: number = FR.b;
  static s: string = FR.t;
  static #x: number = this.#y;
  static b: number = 1;
  static t: string = 'hi';
  static #y: number = 2;
  static get(): string {
    return `${FR.a},${FR.s},${FR.#x},${FR.b},${FR.t},${FR.#y}`;
  }
}
print("static forward ref");
// CHECK-LABEL: static forward ref
print(FR.get());
// CHECK-NEXT: 0,,0,1,hi,2

// The defaults are stored at the top of the scope, but the initializer runs at
// the class declaration, so it sees the effects of preceding code.
let tick: number = 0;
function bump(): number {
  tick = tick + 1;
  return tick;
}
tick = 41;
class Seq {
  static n: number = bump();
}
print(Seq.n);
// CHECK-NEXT: 42

// Methods are installed before any initializer runs, so calling one declared
// later works.
class D {
  static v: number = D.#m();
  static #m(): number {
    return 42;
  }
}
print(D.v);
// CHECK-NEXT: 42

// A field whose type has no representable default is left uninitialized, so a
// forward reference throws while the class is being defined, rather than
// handing out a bogus value.
class E {
  x: number = 3;
}

function defineF(): void {
  class F {
    static early: E = F.late;
    static late: E = new E();
  }
}

try {
  defineF();
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: ReferenceError

class G {
  static late: E = new E();
  static early: E = G.late;
}
print(G.early.x, G.early === G.late);
// CHECK-NEXT: 3 true

// A generic class gets one initializer function per specialization, from a
// cloned AST.
let boxInitCount: number = 0;

class Box<T> {
  static initialized: number = countBoxInit();
  value: T;
  constructor(v: T) {
    this.value = v;
  }
}

function countBoxInit(): number {
  boxInitCount++;
  return boxInitCount;
}

print(new Box<number>(3).value, new Box<string>('s').value);
// CHECK-NEXT: 3 s
print(boxInitCount);
// CHECK-NEXT: 2
