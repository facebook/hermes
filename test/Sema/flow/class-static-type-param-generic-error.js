/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// Generic static methods are typechecked when specialized, which must not see
// the class type parameters either. The method's own type parameters are
// still visible, even when they share a name with the class's.
class Box<T> {
  v: T;
  constructor(v: T) {
    this.v = v;
  }
  @Hermes.final static sig<U>(u: U, t: T): T {
    return t;
  }
  @Hermes.final static body<U>(u: U): void {
    let y: T = (u: any);
  }
  @Hermes.final static own<T>(t: T): T {
    return t;
  }
}

class S extends Box<number> {
  constructor() {
    super(1);
  }
  static q(): void {
    super.sig<string>('x', 5);
    super.body<string>('x');
    super.own<string>('x');
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}class-static-type-param-generic-error.js:20:40: error: ft: class type parameter T cannot be used in a static member
// CHECK-NEXT:  @Hermes.final static sig<U>(u: U, t: T): T {
// CHECK-NEXT:                                       ^
// CHECK-NEXT:{{.*}}class-static-type-param-generic-error.js:20:44: error: ft: class type parameter T cannot be used in a static member
// CHECK-NEXT:  @Hermes.final static sig<U>(u: U, t: T): T {
// CHECK-NEXT:                                           ^
// CHECK-NEXT:{{.*}}class-static-type-param-generic-error.js:24:12: error: ft: class type parameter T cannot be used in a static member
// CHECK-NEXT:    let y: T = (u: any);
// CHECK-NEXT:           ^
// CHECK-NEXT:Emitted 3 errors. exiting.
