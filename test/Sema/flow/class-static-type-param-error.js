/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

'use strict';

// Statics are reached without type arguments, through the class name or
// 'super', so they can't depend on the class type parameters.
class Box<T> {
  static x: T;
// CHECK: {{.*}}class-static-type-param-error.js:[[@LINE-1]]:13: error: ft: class type parameter T cannot be used in a static member
  static s(x: T): void {}
// CHECK: {{.*}}class-static-type-param-error.js:[[@LINE-1]]:15: error: ft: class type parameter T cannot be used in a static member
  v: T;
  constructor(v: T) {
    this.v = v;
  }
}
new Box<number>(1);

// CHECK: Emitted 2 errors. exiting.
