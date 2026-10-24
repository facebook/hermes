/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

// A class without a static SH_call cannot be called without 'new'.
class NoCall {
  x: number;
  constructor(x: number) {
    this.x = x;
  }
}
NoCall(1);
// CHECK: {{.*}}class-sh-call-error.js:17:1: error: ft: callee is not a function

// A non-static SH_call does not enable plain calls.
class Instance {
  SH_call(s: string): string {
    return s;
  }
}
Instance('x');
// CHECK: {{.*}}class-sh-call-error.js:26:1: error: ft: callee is not a function

// Argument types are checked against SH_call's signature.
class C {
  static SH_call(s: string): string {
    return s;
  }
}
C(42);
// CHECK: {{.*}}class-sh-call-error.js:35:3: error: ft: function parameter 's' type mismatch: cannot assign 42 to string

// Argument count is checked against SH_call's signature.
C();
// CHECK: {{.*}}class-sh-call-error.js:39:1: error: ft: function expects 1 arguments, but 0 supplied

// An overloaded static SH_call cannot be called without 'new'; this is
// rejected rather than dispatched.
class Over {
  @Hermes.final @Hermes.overload
  static SH_call(x: number): number {
    return x;
  }
  @Hermes.final @Hermes.overload
  static SH_call(x: boolean): boolean {
    return x;
  }
}
Over(1);
// CHECK: {{.*}}class-sh-call-error.js:54:1: error: ft: overloaded SH_call method not supported

// Dispatch requires the callee to name a class declaration.
const Expr = class {
  static SH_call(): number {
    return 1;
  }
};
Expr();
// CHECK: {{.*}}class-sh-call-error.js:63:1: error: ft: callee is not a function
class Target {
  constructor() {
    new.target();
  }
  static SH_call(): number {
    return 1;
  }
}
// CHECK: {{.*}}class-sh-call-error.js:67:5: error: ft: callee is not a function
