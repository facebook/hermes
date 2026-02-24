/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheck --match-full-lines %s

'use strict';

class A {
    1: string;
}

// CHECK: {{.*}}:13:5: error: ft: property name must be an identifier
// CHECK-NEXT: 1: string;
// CHECK-NEXT: ^~~~~~~~~~
