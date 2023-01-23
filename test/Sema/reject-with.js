/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-ir 2>&1 ) | %FileCheck --match-full-lines %s

with ({a:10})
    print(a);
// CHECK: {{.*}}reject-with.js:[[@LINE-2]]:1: error: with statement is not supported
