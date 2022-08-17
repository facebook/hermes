/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

function *obj() {
  1, {yield} = {};
}
// CHECK: {{.*}}:11:7: error: Unexpected usage of 'yield' as an identifier
// CHECK:   1, {yield} = {};
// CHECK:       ^~~~~
