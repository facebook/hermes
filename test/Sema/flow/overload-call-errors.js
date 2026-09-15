/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

class Foo {
  @Hermes.final @Hermes.overload
  bar(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  bar(x: string): string { return x; }

  // No matching overload: wrong arity.
  testNoMatch(): void {
    this.bar(1, 2, 3);
  }
}

class Bar {
  @Hermes.final @Hermes.overload
  baz(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  baz(x: number): number { return x + 1; }

  // Ambiguous overload: both match.
  testAmbiguous(): void {
    this.baz(42);
  }
}

// Referencing an overloaded method outside a call expression.
class Baz {
  @Hermes.final @Hermes.overload
  qux(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  qux(x: string): string { return x; }

  testRef(): void {
    let f = this.qux;
  }
}

// Same errors apply to private overloaded methods.
class Priv {
  @Hermes.final @Hermes.overload
  #pbar(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  #pbar(x: string): string { return x; }

  testNoMatch(): void {
    this.#pbar(1, 2, 3);
  }
  testRef(): void {
    let f = this.#pbar;
  }
}

// Ambiguous explicit type arguments: multiple generic overloads have the
// same type-parameter count AND the same parameter types after
// specialization, so the call is genuinely ambiguous.
class GenericAmbig {
  @Hermes.final @Hermes.overload
  qux<T>(x: T): T { return x; }
  @Hermes.final @Hermes.overload
  qux<T>(x: T): T { return x; }

  testAmbig(): void {
    this.qux<string>("hi");
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}overload-call-errors.js:18:5: error: ft: no matching overload for call
// CHECK-NEXT:    this.bar(1, 2, 3);
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}overload-call-errors.js:30:5: error: ft: ambiguous call: multiple overloads match
// CHECK-NEXT:    this.baz(42);
// CHECK-NEXT:    ^~~~~~~~~~~~
// CHECK-NEXT:{{.*}}overload-call-errors.js:42:18: error: ft: overloaded method qux cannot be referenced outside a call expression
// CHECK-NEXT:    let f = this.qux;
// CHECK-NEXT:                 ^~~
// CHECK-NEXT:{{.*}}overload-call-errors.js:54:5: error: ft: no matching overload for call
// CHECK-NEXT:    this.#pbar(1, 2, 3);
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}overload-call-errors.js:57:18: error: ft: overloaded method #pbar cannot be referenced outside a call expression
// CHECK-NEXT:    let f = this.#pbar;
// CHECK-NEXT:                 ^~~~~
// CHECK-NEXT:{{.*}}overload-call-errors.js:71:5: error: ft: ambiguous call: multiple overloads match
// CHECK-NEXT:    this.qux<string>("hi");
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 6 errors. exiting.
