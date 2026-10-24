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

// A static field may not be overridden, even with the same type: statics live
// in Variables, so the subclass would get a second storage location.
class B {
  static y: number = 1;
}
class C extends B {
  static y: number = 2;
}
// CHECK: {{.*}}class-static-error.js:24:10: error: ft: cannot override static field y
// CHECK: {{.*}}class-static-error.js:21:10: note: ft: previous declaration of y

// The superclass need not be the immediate parent.
class E {
  static z: number = 1;
}
class F extends E {}
class G extends F {
  static z: number = 2;
}
// CHECK: {{.*}}class-static-error.js:35:10: error: ft: cannot override static field z
// CHECK: {{.*}}class-static-error.js:31:10: note: ft: previous declaration of z

// Duplicate static methods.
class D {
  static foo(): void {}
  static foo(): void {}
}
// CHECK: {{.*}}class-static-error.js:43:10: error: ft: method foo already declared
// CHECK: {{.*}}class-static-error.js:42:3: note: ft: previous declaration of foo
