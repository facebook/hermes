/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes --typed --dump-sema %s 2>&1 ) | %FileCheck --match-full-lines %s

let a1 : number[] = [];
let a2 : string[] = a1;
// CHECK: {{.*}}:[[@LINE-1]]:5: error: {{.*}}

let a3: number[][] = [];
let a4: bool[][] = a3;
// CHECK: {{.*}}:[[@LINE-1]]:5: error: {{.*}}
