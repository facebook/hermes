/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs -dump-bytecode %s | %FileCheck --match-full-lines %s

print('done');

// CHECK: Global String Table:
// CHECK:   s0[ASCII, {{.*}}]: cjs-simple.js

// CHECK: CommonJS Modules:
// CHECK-NEXT:   File ID 0 -> function ID 1
