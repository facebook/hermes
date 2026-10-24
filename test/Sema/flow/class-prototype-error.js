/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

class A {
  foo(): number {
    return 1;
  }
  @Hermes.final finalMethod(): number {
    return 2;
  }
  @Hermes.final get getter(): number {
    return 3;
  }
  @Hermes.final set setter(v: number) {}
  @Hermes.final generic<T>(v: T): T {
    return v;
  }
  @Hermes.final @Hermes.overload over(v: number): number {
    return v;
  }
  @Hermes.final @Hermes.overload over(v: string): string {
    return v;
  }
  #priv(): number {
    return 5;
  }
  usePriv(): number {
    A.prototype.#priv;
    return this.#priv();
  }
}
// CHECK: {{.*}}class-prototype-error.js:34:17: error: ft: property #priv is not stored on the prototype

// Members without a layout slot do not live on the runtime home object.
A.prototype.finalMethod;
// CHECK: {{.*}}class-prototype-error.js:41:13: error: ft: property finalMethod is not stored on the prototype
A.prototype.getter;
// CHECK: {{.*}}class-prototype-error.js:43:13: error: ft: property getter is not stored on the prototype
A.prototype.setter = 1;
// CHECK: {{.*}}class-prototype-error.js:45:13: error: ft: property setter is not stored on the prototype
A.prototype.generic;
// CHECK: {{.*}}class-prototype-error.js:47:13: error: ft: property generic is not stored on the prototype
A.prototype.generic<number>(1);
// CHECK: {{.*}}class-prototype-error.js:49:13: error: ft: property generic is not stored on the prototype
A.prototype.over(1);
// CHECK: {{.*}}class-prototype-error.js:51:13: error: ft: property over is not stored on the prototype

// Unknown property. The second form is reported twice, because the `.call`
// path and the type argument path each resolve the callee.
A.prototype.nonexistent;
// CHECK: {{.*}}class-prototype-error.js:56:13: error: ft: property nonexistent not defined on the prototype object
A.prototype.call<number>(new A());
// CHECK: {{.*}}class-prototype-error.js:58:13: error: ft: property call not defined on the prototype object

// The prototype itself is read-only.
A.prototype = A.prototype;
// CHECK: {{.*}}class-prototype-error.js:62:3: error: ft: cannot assign to prototype
A.prototype++;
// CHECK: {{.*}}class-prototype-error.js:64:3: error: ft: cannot assign to prototype

// Calling a method through the prototype passes the wrong 'this'.
A.prototype.foo();
// CHECK: {{.*}}class-prototype-error.js:68:1: error: ft: 'this' type mismatch

// Methods on the prototype are read-only. IRGen has no lowering for a store to
// a home object, so every syntactic form of write must be rejected here.
A.prototype.foo = A.prototype.foo;
// CHECK: {{.*}}class-prototype-error.js:73:13: error: ft: cannot assign to a prototype method
[A.prototype.foo] = [A.prototype.foo];
// CHECK: {{.*}}class-prototype-error.js:75:14: error: ft: cannot assign to a prototype method
({p: A.prototype.foo} = {p: A.prototype.foo});
// CHECK: {{.*}}class-prototype-error.js:77:18: error: ft: cannot assign to a prototype method
for (A.prototype.foo in {}) {}
// CHECK: {{.*}}class-prototype-error.js:79:18: error: ft: cannot assign to a prototype method
for (A.prototype.foo of [A.prototype.foo]) {}
// CHECK: {{.*}}class-prototype-error.js:81:18: error: ft: cannot assign to a prototype method

// The receiver need not be spelled `A.prototype`.
let alias = A.prototype;
alias.foo = alias.foo;
// CHECK: {{.*}}class-prototype-error.js:86:7: error: ft: cannot assign to a prototype method
