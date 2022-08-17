/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes --basic-block-profiling %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: basic_block_profiler

for (let i = 0; i < 100; i++) {
  var obj = {a: 1, b: 2};
  for (let j = 0; j < 40; j++) {
    obj['p' + i + '|' + j] = i; // cache miss
    obj.p = 2;                  // cache miss
  }
}

// CHECK: {{.*}}profile_index{{.*}}
