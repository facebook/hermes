/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -dump-transformed-ast --pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

a?.b = 3;
// CHECK: {{.*}}:10:1: error: invalid assignment left-hand side
// CHECK: a?.b = 3;
// CHECK: ^~~~
