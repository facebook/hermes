/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheckOrRegen --match-full-lines %s

function* foo() {
  yield of;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "FunctionDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "foo"
// CHECK-NEXT:      },
// CHECK-NEXT:      "params": [],
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "BlockStatement",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ExpressionStatement",
// CHECK-NEXT:            "expression": {
// CHECK-NEXT:              "type": "YieldExpression",
// CHECK-NEXT:              "argument": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "of"
// CHECK-NEXT:              },
// CHECK-NEXT:              "delegate": false
// CHECK-NEXT:            },
// CHECK-NEXT:            "directive": null
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      },
// CHECK-NEXT:      "generator": true,
// CHECK-NEXT:      "async": false
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
