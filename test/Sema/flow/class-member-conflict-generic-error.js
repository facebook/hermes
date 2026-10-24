/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheck --match-full-lines %s

// Specializations are parsed in a second phase, so this conflict is only
// reported once the class is specialized. It must be kept as the only error in
// the file, because the second phase doesn't run if the first one failed.
//
// The conflicting method is dropped from the class, so the specialization is
// not typechecked and this stays a single error.

class A { static foo: number = 1; }
class B<T> extends A { static foo(): number { return 2; } }
new B<number>();
// CHECK: {{.*}}class-member-conflict-generic-error.js:18:31: error: ft: cannot override field foo with a method
// CHECK: Emitted 1 errors. exiting.
