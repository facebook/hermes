/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// Statics are resolved through the class binding, so it must always name
// exactly its class and can't be reassigned in any form.
class D {
  static f: number = 1;
}
class E extends D {}

D = E;
D += 1;
D++;
[D] = [E];
for (D of [E]) {
}

// Reading the binding is fine.
let alias = D;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}class-reassign-error.js:19:1: error: invalid assignment left-hand side
// CHECK-NEXT:D = E;
// CHECK-NEXT:^
// CHECK-NEXT:{{.*}}class-reassign-error.js:20:1: error: invalid assignment left-hand side
// CHECK-NEXT:D += 1;
// CHECK-NEXT:^
// CHECK-NEXT:{{.*}}class-reassign-error.js:21:1: error: invalid operand in update operation
// CHECK-NEXT:D++;
// CHECK-NEXT:^
// CHECK-NEXT:{{.*}}class-reassign-error.js:22:2: error: invalid assignment left-hand side
// CHECK-NEXT:[D] = [E];
// CHECK-NEXT: ^
// CHECK-NEXT:{{.*}}class-reassign-error.js:23:6: error: invalid assignment left-hand side
// CHECK-NEXT:for (D of [E]) {
// CHECK-NEXT:     ^
// CHECK-NEXT:Emitted 5 errors. exiting.
