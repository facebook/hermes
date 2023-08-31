/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: echo "1" "+"{1..10000} | %shermes -dump-sema - >/dev/null
// RUN: (! echo "1" "+"{1..10000} | %shermes -typed -parse-flow -dump-sema - 2>&1 ) | %FileCheck -match-full-lines %s

// CHECK: {{.*}}: error: ft: too many nested expressions/statements/declarations
