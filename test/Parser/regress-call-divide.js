/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast --pretty-json %s | %FileCheckOrRegen %s --match-full-lines

// Make sure the GrammarContext after the call is correct,
// even with a trailing comma.
f(0,) / 1;

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "BinaryExpression",
// CHECK-NEXT:        "left": {
// CHECK-NEXT:          "type": "CallExpression",
// CHECK-NEXT:          "callee": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "f"
// CHECK-NEXT:          },
// CHECK-NEXT:          "arguments": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 0,
// CHECK-NEXT:              "raw": "0"
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        },
// CHECK-NEXT:        "right": {
// CHECK-NEXT:          "type": "NumericLiteral",
// CHECK-NEXT:          "value": 1,
// CHECK-NEXT:          "raw": "1"
// CHECK-NEXT:        },
// CHECK-NEXT:        "operator": "\/"
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
