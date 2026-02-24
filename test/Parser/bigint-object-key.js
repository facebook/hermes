/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheckOrRegen %s --match-full-lines

({ 1_2n: null });

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "ObjectExpression",
// CHECK-NEXT:        "properties": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "Property",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "BigIntLiteral",
// CHECK-NEXT:              "bigint": "12"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NullLiteral"
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "init",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "method": false,
// CHECK-NEXT:            "shorthand": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
