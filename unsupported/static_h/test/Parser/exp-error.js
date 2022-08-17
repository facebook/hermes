/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

+3 ** 2;
// CHECK: {{.*}}:10:1: error: Unary operator before ** must use parens to disambiguate
// CHECK: +3 ** 2;
// CHECK: ^~~~~

delete 3 ** 2;
// CHECK: {{.*}}:15:1: error: Unary operator before ** must use parens to disambiguate
// CHECK: delete 3 ** 2;
// CHECK: ^~~~~~~~~~~
