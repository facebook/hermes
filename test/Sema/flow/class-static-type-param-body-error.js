/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

'use strict';

// Class type parameters are also unavailable in static initializers and
// static method bodies. These are reported in a later phase than member
// signatures, so they are kept apart from class-static-type-param-error.js.
class Box<T> {
  static f: any = (x: T): void => {};
// CHECK: {{.*}}class-static-type-param-body-error.js:[[@LINE-1]]:23: error: ft: class type parameter T cannot be used in a static member
  static t(): void {
    type A = T;
// CHECK: {{.*}}class-static-type-param-body-error.js:[[@LINE-1]]:14: error: ft: class type parameter T cannot be used in a static member
  }
  v: T;
  constructor(v: T) {
    this.v = v;
  }
}
new Box<number>(1);

// CHECK: Emitted 2 errors. exiting.
