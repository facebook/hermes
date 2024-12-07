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

match (a) {

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "MatchStatement",
// CHECK-NEXT:      "argument": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "a"
// CHECK-NEXT:      },
// CHECK-NEXT:      "cases": [

  1 if b: {
    const x = 1;
  },

// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "MatchStatementCase",
// CHECK-NEXT:          "pattern": {
// CHECK-NEXT:            "type": "MatchLiteralPattern",
// CHECK-NEXT:            "literal": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 1,
// CHECK-NEXT:              "raw": "1"
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          "body": {
// CHECK-NEXT:            "type": "BlockStatement",
// CHECK-NEXT:            "body": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "VariableDeclaration",
// CHECK-NEXT:                "kind": "const",
// CHECK-NEXT:                "declarations": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "VariableDeclarator",
// CHECK-NEXT:                    "init": {
// CHECK-NEXT:                      "type": "NumericLiteral",
// CHECK-NEXT:                      "value": 1,
// CHECK-NEXT:                      "raw": "1"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "x"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "guard": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "b"
// CHECK-NEXT:          }
// CHECK-NEXT:        },

  'foo' if f(): {
    const x = 2;
  },

// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "MatchStatementCase",
// CHECK-NEXT:          "pattern": {
// CHECK-NEXT:            "type": "MatchLiteralPattern",
// CHECK-NEXT:            "literal": {
// CHECK-NEXT:              "type": "StringLiteral",
// CHECK-NEXT:              "value": "foo"
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          "body": {
// CHECK-NEXT:            "type": "BlockStatement",
// CHECK-NEXT:            "body": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "VariableDeclaration",
// CHECK-NEXT:                "kind": "const",
// CHECK-NEXT:                "declarations": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "VariableDeclarator",
// CHECK-NEXT:                    "init": {
// CHECK-NEXT:                      "type": "NumericLiteral",
// CHECK-NEXT:                      "value": 2,
// CHECK-NEXT:                      "raw": "2"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "x"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "guard": {
// CHECK-NEXT:            "type": "CallExpression",
// CHECK-NEXT:            "callee": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "f"
// CHECK-NEXT:            },
// CHECK-NEXT:            "arguments": []
// CHECK-NEXT:          }
// CHECK-NEXT:        },

  2 if x < y: {
    const x = 3;
  },
}

// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "MatchStatementCase",
// CHECK-NEXT:          "pattern": {
// CHECK-NEXT:            "type": "MatchLiteralPattern",
// CHECK-NEXT:            "literal": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 2,
// CHECK-NEXT:              "raw": "2"
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          "body": {
// CHECK-NEXT:            "type": "BlockStatement",
// CHECK-NEXT:            "body": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "VariableDeclaration",
// CHECK-NEXT:                "kind": "const",
// CHECK-NEXT:                "declarations": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "VariableDeclarator",
// CHECK-NEXT:                    "init": {
// CHECK-NEXT:                      "type": "NumericLiteral",
// CHECK-NEXT:                      "value": 3,
// CHECK-NEXT:                      "raw": "3"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "x"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "guard": {
// CHECK-NEXT:            "type": "BinaryExpression",
// CHECK-NEXT:            "left": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "x"
// CHECK-NEXT:            },
// CHECK-NEXT:            "right": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "y"
// CHECK-NEXT:            },
// CHECK-NEXT:            "operator": "<"
// CHECK-NEXT:          }
// CHECK-NEXT:        }

// CHECK-NEXT:      ]
// CHECK-NEXT:    }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
