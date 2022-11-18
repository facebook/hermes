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
    yield* (function *() { yield [7,8,9]; })();
}

for (let [a,b,c] of test()) {
    print("[", a, b, c, "]");
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
// CHKAST-NEXT:                "type": "CallExpression",
// CHKAST-NEXT:                "callee": {
// CHKAST-NEXT:                  "type": "FunctionExpression",
// CHKAST-NEXT:                  "id": null,
// CHKAST-NEXT:                  "params": [],
// CHKAST-NEXT:                  "body": {
// CHKAST-NEXT:                    "type": "BlockStatement",
// CHKAST-NEXT:                    "body": [
// CHKAST-NEXT:                      {
// CHKAST-NEXT:                        "type": "ExpressionStatement",
// CHKAST-NEXT:                        "expression": {
// CHKAST-NEXT:                          "type": "YieldExpression",
// CHKAST-NEXT:                          "argument": {
// CHKAST-NEXT:                            "type": "ArrayExpression",
// CHKAST-NEXT:                            "elements": [
// CHKAST-NEXT:                              {
// CHKAST-NEXT:                                "type": "NumericLiteral",
// CHKAST-NEXT:                                "value": 7,
// CHKAST-NEXT:                                "raw": "7"
// CHKAST-NEXT:                              },
// CHKAST-NEXT:                              {
// CHKAST-NEXT:                                "type": "NumericLiteral",
// CHKAST-NEXT:                                "value": 8,
// CHKAST-NEXT:                                "raw": "8"
// CHKAST-NEXT:                              },
// CHKAST-NEXT:                              {
// CHKAST-NEXT:                                "type": "NumericLiteral",
// CHKAST-NEXT:                                "value": 9,
// CHKAST-NEXT:                                "raw": "9"
// CHKAST-NEXT:                              }
// CHKAST-NEXT:                            ],
// CHKAST-NEXT:                            "trailingComma": false
// CHKAST-NEXT:                          },
// CHKAST-NEXT:                          "delegate": false
// CHKAST-NEXT:                        },
// CHKAST-NEXT:                        "directive": null
// CHKAST-NEXT:                      }
// CHKAST-NEXT:                    ]
// CHKAST-NEXT:                  },
// CHKAST-NEXT:                  "generator": true,
// CHKAST-NEXT:                  "async": false
// CHKAST-NEXT:                },
// CHKAST-NEXT:                "arguments": []
// CHKAST-NEXT:              },
// CHKAST-NEXT:              "delegate": true
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "directive": null
// CHKAST-NEXT:          }
// CHKAST-NEXT:        ]
// CHKAST-NEXT:      },
// CHKAST-NEXT:      "generator": true,
// CHKAST-NEXT:      "async": false
// CHKAST-NEXT:    },
// CHKAST-NEXT:    {
// CHKAST-NEXT:      "type": "ForOfStatement",
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
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "a"
// CHKAST-NEXT:                },
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "b"
// CHKAST-NEXT:                },
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "c"
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
// CHKAST-NEXT:                  "type": "StringLiteral",
// CHKAST-NEXT:                  "value": "["
// CHKAST-NEXT:                },
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "a"
// CHKAST-NEXT:                },
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "b"
// CHKAST-NEXT:                },
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "Identifier",
// CHKAST-NEXT:                  "name": "c"
// CHKAST-NEXT:                },
// CHKAST-NEXT:                {
// CHKAST-NEXT:                  "type": "StringLiteral",
// CHKAST-NEXT:                  "value": "]"
// CHKAST-NEXT:                }
// CHKAST-NEXT:              ]
// CHKAST-NEXT:            },
// CHKAST-NEXT:            "directive": null
// CHKAST-NEXT:          }
// CHKAST-NEXT:        ]
// CHKAST-NEXT:      },
// CHKAST-NEXT:      "await": false
// CHKAST-NEXT:    }
// CHKAST-NEXT:  ]
// CHKAST-NEXT:}

// CHKJS:function* test(){
// CHKJS-NEXT:  yield [1, 2, 3];
// CHKJS-NEXT:  yield [2, 3, 4];
// CHKJS-NEXT:  yield* function*(){
// CHKJS-NEXT:    yield [7, 8, 9];
// CHKJS-NEXT:  }();
// CHKJS-NEXT:}
// CHKJS-NEXT:for(let [a, b, c] of test()) {
// CHKJS-NEXT:  print("[", a, b, c, "]");
// CHKJS-NEXT:}
