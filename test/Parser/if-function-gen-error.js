/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast --pretty-json %s 2>&1 ) | %FileCheck %s --match-full-lines

if (x) function* f() {}

// CHECK:{{.*}}:10:8: error: Functions in if statements cannot be generator/async
// CHECK-NEXT:if (x) function* f() {}
// CHECK-NEXT:       ^
