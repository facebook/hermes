/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -dump-ast --pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

new obj ?. (arg);
// // CHECK: {{.*}}:10:9: error: Constructor calls may not contain an optional chain
// // CHECK: new obj ?. (arg);
// // CHECK:         ^~

new obj ?. prop();
// CHECK: {{.*}}:15:9: error: Constructor calls may not contain an optional chain
// CHECK: new obj ?. prop();
// CHECK:         ^~

a?.b.c`abc`;
// CHECK: {{.*}}:20:7: error: invalid use of tagged template literal in optional chain
// CHECK: a?.b.c`abc`;
// CHECK:       ^~~~~
// CHECK: {{.*}}:20:1: note: location of optional chain
// CHECK: a?.b.c`abc`;
// CHECK: ^~~~~~

a?.b.c
`abc`;
// CHECK: {{.*}}:29:1: error: invalid use of tagged template literal in optional chain
// CHECK: `abc`;
// CHECK: ^~~~~
// CHECK: {{.*}}:28:1: note: location of optional chain
// CHECK: a?.b.c
// CHECK: ^~~~~~
