/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast --pretty-json %s 2>&1 ) | %FileCheck %s --match-full-lines

'use strict'
if (x) function f() {} else function f() {}

// CHECK:{{.*}}:11:8: error: In strict mode, functions cannot be declared in if statements
// CHECK-NEXT:if (x) function f() {} else function f() {}
// CHECK-NEXT:       ^
