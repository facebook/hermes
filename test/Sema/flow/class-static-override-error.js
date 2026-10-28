/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Statics are resolved at compile time, including through 'this' in an
// inherited static method, so a subclass can't redeclare a static method:
// the redeclaration would only be reached through the subclass name.
class F {
  static m(): number { return 1; }
}
class G extends F {
  static m(): number { return 2; }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}class-static-override-error.js:17:10: error: ft: cannot override static method m
// CHECK-NEXT:  static m(): number { return 2; }
// CHECK-NEXT:         ^
// CHECK-NEXT:{{.*}}class-static-override-error.js:14:10: note: ft: previous declaration of m
// CHECK-NEXT:  static m(): number { return 1; }
// CHECK-NEXT:         ^
// CHECK-NEXT:Emitted 1 errors. exiting.
