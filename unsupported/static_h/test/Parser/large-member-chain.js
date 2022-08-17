/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! echo "a" ".b"{1..10000} | %hermesc -dump-ast - 2>&1 ) | %FileCheck -match-full-lines %s

// CHECK: {{.*}}: error: Too many nested expressions/statements/declarations
