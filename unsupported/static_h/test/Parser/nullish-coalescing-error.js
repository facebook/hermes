/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1) | %FileCheck %s --match-full-lines

a ?? b && c;
// CHECK: {{.*}}:10:1: error: Mixing '??' with '&&' or '||' requires parentheses
// CHECK: a ?? b && c;
// CHECK: ^~~~~~~~~~~

a ?? b || c;
// CHECK: {{.*}}:15:1: error: Mixing '??' with '&&' or '||' requires parentheses
// CHECK: a ?? b || c;
// CHECK: ^~~~~~~~~~~

a || b ?? c;
// CHECK: {{.*}}:20:1: error: Mixing '??' with '&&' or '||' requires parentheses
// CHECK: a || b ?? c;
// CHECK: ^~~~~~~~~~~
