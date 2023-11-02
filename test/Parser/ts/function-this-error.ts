/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-ts -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

type A = (this?: string) => void;
// CHECK: {{.*}}:10:11: error: 'this' constraint may not be optional
// CHECK-NEXT: type A = (this?: string) => void;
// CHECK-NEXT:           ^~~~
