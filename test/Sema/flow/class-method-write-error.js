/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

// Methods are readonly. IRGen looks a stored-to property up in the instance
// class, where methods do not live, so every syntactic form of write must be
// rejected here.

class A {
  foo(): number {
    return 1;
  }
  static stat(): number {
    return 2;
  }
  @Hermes.final fin(): number {
    return 3;
  }
  writeThis() {
    this.foo = this.foo;
    // CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:10: error: ft: cannot assign to readonly property foo
  }
}

function f(): number {
  return 4;
}

let a = new A();

// Every write form through an instance.
a.foo = f;
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:3: error: ft: cannot assign to readonly property foo
[a.foo] = [f];
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:4: error: ft: cannot assign to readonly property foo
({p: a.foo} = {p: f});
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:8: error: ft: cannot assign to readonly property foo
for (a.foo in {}) {}
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:8: error: ft: cannot assign to readonly property foo
for (a.foo of [f]) {}
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:8: error: ft: cannot assign to readonly property foo
a.foo += 1;
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:3: error: ft: cannot assign to readonly property foo
a.foo++;
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:3: error: ft: cannot assign to readonly property foo

// Writes through a subclass receiver, including `super`.
class B extends A {
  writeSuper() {
    super.foo = f;
    // CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:11: error: ft: cannot assign to readonly property foo
  }
}
let b = new B();
b.foo = f;
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:3: error: ft: cannot assign to readonly property foo

// Final methods live in a class-scope Variable shared by all instances; a
// write compiles without this check, silently rebinding it for everyone.
a.fin = f;
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:3: error: ft: cannot assign to readonly property fin

// Static methods likewise live in a Variable shared by every call site.
A.stat = f;
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:3: error: ft: cannot assign to readonly property stat

// A write is still rejected when the value is also ill-typed. Only the
// readonly error is reported, because the property resolves to `any`.
a.foo = 3;
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:3: error: ft: cannot assign to readonly property foo

// `delete` reports only its own error, not a readonly violation.
delete a.foo;
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:1: error: ft: cannot delete property of typed object
// CHECK-NOT: readonly
delete A.stat;
// CHECK: {{.*}}class-method-write-error.js:[[@LINE-1]]:1: error: ft: cannot delete property of typed object
// CHECK-NOT: readonly
