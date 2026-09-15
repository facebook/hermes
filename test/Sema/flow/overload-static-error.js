/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheck --match-full-lines %s

class Foo {
  @Hermes.final @Hermes.overload
  static bar(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  static bar(x: string): string { return x; }
}

class Bar {
  @Hermes.final @Hermes.overload
  static baz(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  static baz(x: number): string { return ""; }
}

class Baz {
  @Hermes.final @Hermes.overload
  static qux(x: number): number { return x; }
  @Hermes.final @Hermes.overload
  static qux(x: string): string { return x; }
}

class Test {
  // No matching overload for static call.
  static testNoMatch(): void {
    Foo.bar(true);
  }
  // Ambiguous overload for static call.
  static testAmbiguous(): void {
    Bar.baz(1);
  }
  // Referencing overloaded static method outside a call expression.
  static testRef(): void {
    let f = Baz.qux;
  }
}

// CHECK: {{.*}}overload-static-error.js:34:5: error: ft: no matching overload for call
// CHECK-NEXT:    Foo.bar(true);
// CHECK-NEXT:    ^~~~~~~~~~~~~
// CHECK: {{.*}}overload-static-error.js:38:5: error: ft: ambiguous call: multiple overloads match
// CHECK-NEXT:    Bar.baz(1);
// CHECK-NEXT:    ^~~~~~~~~~
// CHECK: {{.*}}overload-static-error.js:42:17: error: ft: overloaded method qux cannot be referenced outside a call expression
// CHECK-NEXT:    let f = Baz.qux;
// CHECK-NEXT:                ^~~
