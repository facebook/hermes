/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheck %s --match-full-lines

// Non-Array rest param type must be an error.
// CHECK: {{.*}}:12:21: error: ft: rest parameter type must be Array<T>
function bad(...args: string): void {}
