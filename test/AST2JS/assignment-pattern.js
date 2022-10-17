/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc --dump-ast %s | %FileCheckOrRegen --match-full-lines %s --check-prefix=CHKAST
// RUN: %hermesc --dump-js %s | %FileCheckOrRegen --match-full-lines %s --check-prefix=CHKJS

function *test() {
    yield [1,2,3];
    yield [2,3,4];
    yield [7,8,9];
    yield [0,0,0];
}

var d;
for (let [a = d] in test()) {
    print(a);
}

// Auto-generated content below. Please do not modify manually.

// CHKAST:{
// CHKAST-NEXT:  "type": "Program",
// CHKAST-NEXT:  "body": [
// CHKAST-NEXT:    {
// CHKAST-NEXT:      "type": "FunctionDeclaration",
// CHKAST-NEXT:      "id": {
// CHKAST-NEXT:        "type": "Identifier",
// CHKAST-NEXT:        "name": "test"
// CHKAST-NEXT:      },
// CHKAST-NEXT:      "params": [],
// CHKAST-NEXT:      "body": {
// CHKAST-NEXT:        "type": "BlockStatement",
// CHKAST-NEXT:        "body": [
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ExpressionStatement",
// CHKAST-NEXT:            "expression": {
// CHKAST-NEXT:              "type": "YieldExpression",
// CHKAST-NEXT:              "argument": {
// CHKAST-NEXT:                "type": "ArrayExpression",
// CHKAST-NEXT:                "elements": [
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 1,
// CHKAST-NEXT:                    "raw": "1"
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 2,
// CHKAST-NEXT:                    "raw": "2"
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 3,
// CHKAST-NEXT:                    "raw": "3"
// CHKAST-NEXT:                  }
// CHKAST-NEXT:                ],
// CHKAST-NEXT:                "trailingComma": false
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "delegate": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "directive": null
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ExpressionStatement",
// CHKAST-NEXT:            "expression": {
// CHKAST-NEXT:              "type": "YieldExpression",
// CHKAST-NEXT:              "argument": {
// CHKAST-NEXT:                "type": "ArrayExpression",
// CHKAST-NEXT:                "elements": [
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 2,
// CHKAST-NEXT:                    "raw": "2"
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 3,
// CHKAST-NEXT:                    "raw": "3"
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 4,
// CHKAST-NEXT:                    "raw": "4"
// CHKAST-NEXT:                  }
// CHKAST-NEXT:                ],
// CHKAST-NEXT:                "trailingComma": false
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "delegate": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "directive": null
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ExpressionStatement",
// CHKAST-NEXT:            "expression": {
// CHKAST-NEXT:              "type": "YieldExpression",
// CHKAST-NEXT:              "argument": {
// CHKAST-NEXT:                "type": "ArrayExpression",
// CHKAST-NEXT:                "elements": [
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 7,
// CHKAST-NEXT:                    "raw": "7"
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 8,
// CHKAST-NEXT:                    "raw": "8"
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 9,
// CHKAST-NEXT:                    "raw": "9"
// CHKAST-NEXT:                  }
// CHKAST-NEXT:                ],
// CHKAST-NEXT:                "trailingComma": false
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "delegate": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "directive": null
// CHKAST-NEXT:          },
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ExpressionStatement",
// CHKAST-NEXT:            "expression": {
// CHKAST-NEXT:              "type": "YieldExpression",
// CHKAST-NEXT:              "argument": {
// CHKAST-NEXT:                "type": "ArrayExpression",
// CHKAST-NEXT:                "elements": [
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 0,
// CHKAST-NEXT:                    "raw": "0"
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 0,
// CHKAST-NEXT:                    "raw": "0"
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  {
// CHKAST-NEXT:                    "type": "NumericLiteral",
// CHKAST-NEXT:                    "value": 0,
// CHKAST-NEXT:                    "raw": "0"
// CHKAST-NEXT:                  }
// CHKAST-NEXT:                ],
// CHKAST-NEXT:                "trailingComma": false
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "delegate": false
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "directive": null
// CHKAST-NEXT:          }
// CHKAST-NEXT:        ]
// CHKAST-NEXT:      },
// CHKAST-NEXT:      "generator": true,
// CHKAST-NEXT:      "async": false
// CHKAST-NEXT:    },
// CHKAST-NEXT:    {
// CHKAST-NEXT:      "type": "VariableDeclaration",
// CHKAST-NEXT:      "kind": "var",
// CHKAST-NEXT:      "declarations": [
// CHKAST-NEXT:        {
// CHKAST-NEXT:          "type": "VariableDeclarator",
// CHKAST-NEXT:          "init": null,
// CHKAST-NEXT:          "id": {
// CHKAST-NEXT:            "type": "Identifier",
// CHKAST-NEXT:            "name": "d"
// CHKAST-NEXT:          }
// CHKAST-NEXT:        }
// CHKAST-NEXT:      ]
// CHKAST-NEXT:    },
// CHKAST-NEXT:    {
// CHKAST-NEXT:      "type": "ForInStatement",
// CHKAST-NEXT:      "left": {
// CHKAST-NEXT:        "type": "VariableDeclaration",
// CHKAST-NEXT:        "kind": "let",
// CHKAST-NEXT:        "declarations": [
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "VariableDeclarator",
// CHKAST-NEXT:            "init": null,
// CHKAST-NEXT:            "id": {
// CHKAST-NEXT:              "type": "ArrayPattern",
// CHKAST-NEXT:              "elements": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "AssignmentPattern",
// CHKAST-NEXT:                  "left": {
// CHKAST-NEXT:                    "type": "Identifier",
// CHKAST-NEXT:                    "name": "a"
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  "right": {
// CHKAST-NEXT:                    "type": "Identifier",
// CHKAST-NEXT:                    "name": "d"
// CHKAST-NEXT:                  }
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ]
// CHKAST-NEXT:            }
// CHKAST-NEXT:          }
// CHKAST-NEXT:        ]
// CHKAST-NEXT:      },
// CHKAST-NEXT:      "right": {
// CHKAST-NEXT:        "type": "CallExpression",
// CHKAST-NEXT:        "callee": {
// CHKAST-NEXT:          "type": "Identifier",
// CHKAST-NEXT:          "name": "test"
// CHKAST-NEXT:        },
// CHKAST-NEXT:        "arguments": []
// CHKAST-NEXT:      },
// CHKAST-NEXT:      "body": {
// CHKAST-NEXT:        "type": "BlockStatement",
// CHKAST-NEXT:        "body": [
// CHKAST-NEXT:          {
// CHKAST-NEXT:            "type": "ExpressionStatement",
// CHKAST-NEXT:            "expression": {
// CHKAST-NEXT:              "type": "CallExpression",
// CHKAST-NEXT:              "callee": {
// CHKAST-NEXT:                "type": "Identifier",
// CHKAST-NEXT:                "name": "print"
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "arguments": [
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "a"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ]
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "directive": null
// CHKAST-NEXT:          }
// CHKAST-NEXT:        ]
// CHKAST-NEXT:      }
// CHKAST-NEXT:    }
// CHKAST-NEXT:  ]
// CHKAST-NEXT:}

// CHKJS:function* test(){
// CHKJS-NEXT:  yield [1, 2, 3];
// CHKJS-NEXT:  yield [2, 3, 4];
// CHKJS-NEXT:  yield [7, 8, 9];
// CHKJS-NEXT:  yield [0, 0, 0];
// CHKJS-NEXT:}
// CHKJS-NEXT:var d;
// CHKJS-NEXT:for(let [a = d] in test()) {
// CHKJS-NEXT:  print(a);
// CHKJS-NEXT:}
