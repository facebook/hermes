/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

(async async() => 3);
// CHECK: {{.*}}:10:8: error: invalid arrow function parameter list
// CHECK-NEXT: (async async() => 3);
// CHECK-NEXT:        ^~~~~~~
