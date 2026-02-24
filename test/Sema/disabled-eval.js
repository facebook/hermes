/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -enable-eval=false -dump-sema -fno-std-globals %s 2>&1 | %FileCheck %s --match-full-lines
// RUN: %shermes -enable-eval=false -dump-sema %s 2>&1 | %FileCheck %s --match-full-lines

eval("print(1)");
//CHECK: {{.*}}disabled-eval.js:[[@LINE-1]]:1: warning: eval() is disabled at runtime
