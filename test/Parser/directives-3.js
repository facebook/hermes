/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast %s | %FileCheck --match-full-lines %s
"two \
lines";

// CHECK:      {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "two lines"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": "two \\\nlines"
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
