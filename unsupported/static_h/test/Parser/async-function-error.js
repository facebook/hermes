/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

a = async((x)) => 3;
// CHECK: {{.*}}:10:12: error: parentheses are not allowed around parameters
// CHECK: a = async((x)) => 3;
// CHECK:            ^

({async f: 3});
// CHECK: {{.*}}:15:10: error: '(' expected in method definition
// CHECK: ({async f: 3});
// CHECK:   ~~~~~~~^
