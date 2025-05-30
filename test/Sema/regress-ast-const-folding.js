/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-transformed-ast %s | %FileCheckOrRegen %s --match-full-lines

// Test for a regression where AST constant folding created a malformed AST.

for (w of (1 + 1)) {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ForOfStatement",
// CHECK-NEXT:      "left": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "w"
// CHECK-NEXT:      },
// CHECK-NEXT:      "right": {
// CHECK-NEXT:        "type": "NumericLiteral",
// CHECK-NEXT:        "value": 2,
// CHECK-NEXT:        "raw": "1 + 1"
// CHECK-NEXT:      },
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "BlockStatement",
// CHECK-NEXT:        "body": []
// CHECK-NEXT:      },
// CHECK-NEXT:      "await": false
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
