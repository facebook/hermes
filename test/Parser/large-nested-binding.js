/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! printf '%0.s[' {1..100000} | %hermesc -dump-ast - 2>&1 ) | %FileCheck -match-full-lines %s

// CHECK: {{.*}}Too many nested expressions/statements/declarations{{.*}}
