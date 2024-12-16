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

const e = match (x) {

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
// CHECK-NEXT:              "name": "x"
// CHECK-NEXT:            },
// CHECK-NEXT:            "cases": [

  "s": 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchLiteralPattern",
// CHECK-NEXT:                  "literal": {
// CHECK-NEXT:                    "type": "StringLiteral",
// CHECK-NEXT:                    "value": "s"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  true: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchLiteralPattern",
// CHECK-NEXT:                  "literal": {
// CHECK-NEXT:                    "type": "BooleanLiteral",
// CHECK-NEXT:                    "value": true
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  null: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchLiteralPattern",
// CHECK-NEXT:                  "literal": {
// CHECK-NEXT:                    "type": "NullLiteral"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  3: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchLiteralPattern",
// CHECK-NEXT:                  "literal": {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 3,
// CHECK-NEXT:                    "raw": "3"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  4n: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchLiteralPattern",
// CHECK-NEXT:                  "literal": {
// CHECK-NEXT:                    "type": "BigIntLiteral",
// CHECK-NEXT:                    "bigint": "4n"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  +5: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchUnaryPattern",
// CHECK-NEXT:                  "argument": {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 5,
// CHECK-NEXT:                    "raw": "5"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "operator": "+"
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  -6: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchUnaryPattern",
// CHECK-NEXT:                  "argument": {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 6,
// CHECK-NEXT:                    "raw": "6"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "operator": "-"
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  +7n: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchUnaryPattern",
// CHECK-NEXT:                  "argument": {
// CHECK-NEXT:                    "type": "BigIntLiteral",
// CHECK-NEXT:                    "bigint": "7n"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "operator": "+"
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  -8n: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchUnaryPattern",
// CHECK-NEXT:                  "argument": {
// CHECK-NEXT:                    "type": "BigIntLiteral",
// CHECK-NEXT:                    "bigint": "8n"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "operator": "-"
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  y: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchIdentifierPattern",
// CHECK-NEXT:                  "id": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "y"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  const y: y,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchBindingPattern",
// CHECK-NEXT:                  "id": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "y"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "kind": "const"
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "y"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  let y: y,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchBindingPattern",
// CHECK-NEXT:                  "id": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "y"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "kind": "let"
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "y"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  var y: y,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchBindingPattern",
// CHECK-NEXT:                  "id": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "y"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "kind": "var"
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "y"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  ('s'): 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchLiteralPattern",
// CHECK-NEXT:                  "literal": {
// CHECK-NEXT:                    "type": "StringLiteral",
// CHECK-NEXT:                    "value": "s"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  _: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchWildcardPattern"
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
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
