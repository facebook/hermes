/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

// Duplicate static fields.
class A {
  static x: number = 1;
  static x: number = 2;
}
// CHECK: {{.*}}class-static-error.js:13:10: error: ft: field x already declared
// CHECK: {{.*}}class-static-error.js:12:3: note: ft: previous declaration of x

// Incompatible static override.
class B {
  static y: number = 1;
}
class C extends B {
  static y: string = "bad";
}
// CHECK: {{.*}}class-static-error.js:23:3: error: ft: incompatible field type for override

// Duplicate static methods.
class D {
  static foo(): void {}
  static foo(): void {}
}
// CHECK: {{.*}}class-static-error.js:30:10: error: ft: method foo already declared
// CHECK: {{.*}}class-static-error.js:29:3: note: ft: previous declaration of foo
