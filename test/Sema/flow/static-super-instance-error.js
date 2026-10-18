/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheck --match-full-lines %s

class Base {
  value: number;
}

class Child extends Base {
  static test(): number {
    return super.value;
  }
}

// CHECK:{{.*}}static-super-instance-error.js:16:18: error: ft: static property value not defined in class Base
// CHECK-NEXT:    return super.value;
// CHECK-NEXT:                 ^~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
