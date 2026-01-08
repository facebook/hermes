/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

// Destructuring pattern not allowed with 'using' in for-of.
for (using {x} of y);
// CHECK: {{.*}}:11:12: error: ';' or 'in' expected inside 'for'
// CHECK-NEXT: for (using {x} of y);
// CHECK-NEXT: ~~~~~~~~~~~^
