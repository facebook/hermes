/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc %s -dump-ast 2>&1 ) | %FileCheck %s

declare var x;
// CHECK: {{.*}}:10:9: error: ';' expected
// CHECK-NEXT: declare var x;
// CHECK-NEXT:         ^
