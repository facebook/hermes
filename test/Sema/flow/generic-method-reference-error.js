/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// A generic method has no concrete type until a call site specializes it, and
// no Decl is created for it, so referencing one outside a direct call has
// nothing to resolve to.
// Kept separate from generic-method-error.js: these errors come from
// typechecking bodies, which is skipped once a class fails to parse.

class P {
  @Hermes.final
  static gs<U>(x: U): U { return x; }
  @Hermes.final
  gi<U>(x: U): U { return x; }
}

class C extends P {
  static viaSuperStatic(): any { return super.gs; }
  viaSuperInstance(): any { return super.gi; }
}

// Direct calls remain legal.
function ok(c: C): string { return C.gs<string>('a') + c.gi<string>('b'); }

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-method-reference-error.js:24:47: error: ft: generic method gs cannot be referenced outside a call expression
// CHECK-NEXT:  static viaSuperStatic(): any { return super.gs; }
// CHECK-NEXT:                                              ^~
// CHECK-NEXT:{{.*}}generic-method-reference-error.js:25:42: error: ft: generic method gi cannot be referenced outside a call expression
// CHECK-NEXT:  viaSuperInstance(): any { return super.gi; }
// CHECK-NEXT:                                         ^~
// CHECK-NEXT:Emitted 2 errors. exiting.
