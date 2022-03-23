/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

"",p=B"";
// CHECK: {{.*}}:10:7: error: unexpected token after assignment expression
 // CHECK: "",p=B"";
// CHECK:       ^
