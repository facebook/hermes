/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheck %s --match-full-lines

// Function type annotation with non-array rest param.
// CHECK: {{.*}}:12:23: error: ft: rest parameter type must be Array<T>
type Bad1 = (...rest: string) => void;

// CHECK: {{.*}}:15:23: error: ft: rest parameter type must be Array<T>
type Bad2 = (...rest: number) => void;
