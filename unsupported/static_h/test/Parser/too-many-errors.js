/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ( ! %hermesc -dump-ir -ferror-limit=2 %s 2>&1 ) | %FileCheck --match-full-lines %s

1.a2
//CHECK: {{.*}}too-many-errors.js:10:1: error: invalid numeric literal
//CHECK-NEXT: 1.a2
//CHECK-NEXT: ^~~~

1.a3
//CHECK: {{.*}}too-many-errors.js:15:1: error: invalid numeric literal
//CHECK-NEXT: 1.a3
//CHECK-NEXT: ^~~~
//CHECK-NEXT: <unknown>:0: error: too many errors emitted

1.a4
1.a5
