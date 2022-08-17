/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast %s 2>&1 ) | %FileCheck --match-full-lines %s

(async await => 3);
// CHECK: {{.*}}:10:8: error: Unexpected usage of 'await' as an identifier
// CHECK: (async await => 3);
// CHECK:        ^~~~~

(async (await) => 3);
// CHECK: {{.*}}:15:9: error: Unexpected usage of 'await' as an identifier
// CHECK: (async (await) => 3);
// CHECK:         ^~~~~

(async function() {
  let await = 3;
});
// CHECK: {{.*}}:21:7: error: Unexpected usage of 'await' as an identifier
// CHECK:   let await = 3;
// CHECK:       ^~~~~
