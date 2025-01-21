/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-flow-match -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

const e = match (a) {

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "MatchExpression",
// CHECK-NEXT:            "argument": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "a"
// CHECK-NEXT:            },
// CHECK-NEXT:            "cases": [

  1 if b: true,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchLiteralPattern",
// CHECK-NEXT:                  "literal": {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 1,
// CHECK-NEXT:                    "raw": "1"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BooleanLiteral",
// CHECK-NEXT:                  "value": true
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "b"
// CHECK-NEXT:                }
// CHECK-NEXT:              }

};

// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "e"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
