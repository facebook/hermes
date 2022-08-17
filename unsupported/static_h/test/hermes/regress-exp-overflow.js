/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: echo "1" "**"{1..5000} | (! %hermes - 2>&1 ) | %FileCheck --match-full-lines %s

// CHECK: <stdin>:1:{{.*}}: error: Too many nested expressions/statements/declarations
