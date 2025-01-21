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

  1: true,

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
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  'foo': false,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchLiteralPattern",
// CHECK-NEXT:                  "literal": {
// CHECK-NEXT:                    "type": "StringLiteral",
// CHECK-NEXT:                    "value": "foo"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BooleanLiteral",
// CHECK-NEXT:                  "value": false
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  2: {obj: 'literal'},

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchLiteralPattern",
// CHECK-NEXT:                  "literal": {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 2,
// CHECK-NEXT:                    "raw": "2"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "ObjectExpression",
// CHECK-NEXT:                  "properties": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "Property",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "obj"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "value": {
// CHECK-NEXT:                        "type": "StringLiteral",
// CHECK-NEXT:                        "value": "literal"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "kind": "init",
// CHECK-NEXT:                      "computed": false,
// CHECK-NEXT:                      "method": false,
// CHECK-NEXT:                      "shorthand": false
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },

};

// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "e"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
