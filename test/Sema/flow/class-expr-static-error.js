/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

'use strict';

const C = class {
  static n: number = 1;
// CHECK: {{.*}}class-expr-static-error.js:[[@LINE-1]]:3: error: ft: static members are not supported in class expressions
  static m(): number {
// CHECK: {{.*}}class-expr-static-error.js:[[@LINE-1]]:3: error: ft: static members are not supported in class expressions
    return 2;
  }
  x: number = 3;
};

// Private and final methods live in Variables too.
const D = class {
  #p(): number {
// CHECK: {{.*}}class-expr-static-error.js:[[@LINE-1]]:3: error: ft: private methods are not supported in class expressions
    return 1;
  }
  @Hermes.final f(): number {
// CHECK: {{.*}}class-expr-static-error.js:[[@LINE-1]]:3: error: ft: final methods are not supported in class expressions
    return this.#p();
  }
  m(): number {
    return 2;
  }
};

// Typed private fields aren't supported either.
const E = class {
  #x: number = 3;
// CHECK: {{.*}}class-expr-static-error.js:[[@LINE-1]]:3: error: ft: private fields are not supported in class expressions
};

// CHECK: Emitted 5 errors. exiting.
