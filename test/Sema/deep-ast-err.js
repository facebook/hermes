/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: echo "x" "+"{1..10000} | %shermes -dump-sema - >/dev/null
// RUN: (! echo "x" "+"{1..10000} | %shermes -typed -parse-flow -dump-sema - 2>&1 ) | %FileCheck -match-full-lines %s
// RUN: (! echo "x" "+"{1..10000} | %shermes -typed -parse-ts -dump-sema - 2>&1 ) | %FileCheck -match-full-lines %s --check-prefix=CHECK-TS
// We need to skip this test on Linux with ASAN due to stack overflow (building
// with GCC).
// UNSUPPORTED: linux && asan

// CHECK: {{.*}}: error: ft: too many nested expressions/statements/declarations
// CHECK-TS: {{.*}}: error: ts2flow: too many nested expressions/statements/declarations
