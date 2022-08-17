/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s -dump-ast 2>&1 ) | %FileCheck %s

export { a as+ }

// CHECK: {{.*}}:10:14: error: 'identifier' expected in export clause
// CHECK-NEXT: export { a as+ }
// CHECK-NEXT:        ~~~~~~^
// CHECK-NOT: Assertion failed: {{.*}}
