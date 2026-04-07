/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Unknown @Hermes decorator.
class A {
  @Hermes.unknown
  foo(): void {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}unknown-decorator-error.js:12:3: error: ft: unknown @Hermes decorator
// CHECK-NEXT:  @Hermes.unknown
// CHECK-NEXT:  ^~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
