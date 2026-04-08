/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

(class C {
  async tryParse(text) {
  }
});

// CHECK-LABEL: {{.*}}class-expr-error.js:11:3: error: ft: async/generator methods unsupported
// CHECK-NEXT:   async tryParse(text) {
// CHECK-NEXT:   ^
