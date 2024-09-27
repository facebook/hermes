/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -dump-ast --pretty-json %s 2>&1) | %FileCheckOrRegen %s --match-full-lines

// Printing the warning here was failing due to incorrect location setting.
#do\

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}private-escape-error.js:11:4: error: invalid Unicode escape
// CHECK-NEXT:#do\
// CHECK-NEXT:   ^
// CHECK-NEXT:{{.*}}private-escape-error.js:11:4: error: Unicode escape \ufffd is not a valid identifier codepoint
// CHECK-NEXT:#do\
// CHECK-NEXT:   ^
// CHECK-NEXT:{{.*}}private-escape-error.js:11:1: warning: scanning identifier with unicode escape as reserved word
// CHECK-NEXT:#do\
// CHECK-NEXT:^~~~
// CHECK-NEXT:{{.*}}private-escape-error.js:11:1: error: Private name can only be used as left-hand side of `in` expression
// CHECK-NEXT:#do\
// CHECK-NEXT:^~~~
// CHECK-NEXT:Emitted 3 errors. exiting.
