/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1 ) | %FileCheck --match-full-lines %s

"use strict";

({ x: let = 3 } = {});
// CHECK: {{.*}}:12:7: error: Invalid use of strict mode reserved word as binding identifier
// CHECK-NEXT: ({ x: let = 3 } = {});
// CHECK-NEXT:       ^~~

({ x: let } = {});
// CHECK: {{.*}}:17:7: error: Invalid use of strict mode reserved word as binding identifier
// CHECK-NEXT: ({ x: let } = {});
// CHECK-NEXT:       ^~~
