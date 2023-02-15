/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1 ) | %FileCheck --match-full-lines %s

for (async of 1);
// CHECK: {{.*}}:10:15: error: ';' or 'in' expected inside 'for'
// CHECK-NEXT: for (async of 1);
// CHECK-NEXT: ~~~~~~~~~~~~~~^
