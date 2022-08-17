/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

//CHECK: {
//CHECK-NEXT:    "type": "Program",
//CHECK-NEXT:    "body": [

var t1 = a => 1;
//CHECK-NEXT:      {
//CHECK-NEXT:        "type": "VariableDeclaration",
//CHECK-NEXT:        "kind": "var",
//CHECK-NEXT:        "declarations": [
//CHECK-NEXT:          {
//CHECK-NEXT:            "type": "VariableDeclarator",
//CHECK-NEXT:            "init": {
//CHECK-NEXT:              "type": "ArrowFunctionExpression",
//CHECK-NEXT:              "id": null,
//CHECK-NEXT:              "params": [
//CHECK-NEXT:                {
//CHECK-NEXT:                  "type": "Identifier",
//CHECK-NEXT:                  "name": "a"
//CHECK-NEXT:                }
//CHECK-NEXT:              ],
//CHECK-NEXT:              "body": {
//CHECK-NEXT:                "type": "NumericLiteral",
//CHECK-NEXT:                "value": 1,
//CHECK-NEXT:                "raw": "1"
//CHECK-NEXT:              },
//CHECK-NEXT:              "expression": true,
//CHECK-NEXT:              "async": false
//CHECK-NEXT:            },
//CHECK-NEXT:            "id": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "t1"
//CHECK-NEXT:            }
//CHECK-NEXT:          }
//CHECK-NEXT:        ]
//CHECK-NEXT:      },

var t2 = (a) => 1;
//CHECK-NEXT:      {
//CHECK-NEXT:        "type": "VariableDeclaration",
//CHECK-NEXT:        "kind": "var",
//CHECK-NEXT:        "declarations": [
//CHECK-NEXT:          {
//CHECK-NEXT:            "type": "VariableDeclarator",
//CHECK-NEXT:            "init": {
//CHECK-NEXT:              "type": "ArrowFunctionExpression",
//CHECK-NEXT:              "id": null,
//CHECK-NEXT:              "params": [
//CHECK-NEXT:                {
//CHECK-NEXT:                  "type": "Identifier",
//CHECK-NEXT:                  "name": "a"
//CHECK-NEXT:                }
//CHECK-NEXT:              ],
//CHECK-NEXT:              "body": {
//CHECK-NEXT:                "type": "NumericLiteral",
//CHECK-NEXT:                "value": 1,
//CHECK-NEXT:                "raw": "1"
//CHECK-NEXT:              },
//CHECK-NEXT:              "expression": true,
//CHECK-NEXT:              "async": false
//CHECK-NEXT:            },
//CHECK-NEXT:            "id": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "t2"
//CHECK-NEXT:            }
//CHECK-NEXT:          }
//CHECK-NEXT:        ]
//CHECK-NEXT:      },

var t3 = a => { return 20; }
//CHECK-NEXT:      {
//CHECK-NEXT:        "type": "VariableDeclaration",
//CHECK-NEXT:        "kind": "var",
//CHECK-NEXT:        "declarations": [
//CHECK-NEXT:          {
//CHECK-NEXT:            "type": "VariableDeclarator",
//CHECK-NEXT:            "init": {
//CHECK-NEXT:              "type": "ArrowFunctionExpression",
//CHECK-NEXT:              "id": null,
//CHECK-NEXT:              "params": [
//CHECK-NEXT:                {
//CHECK-NEXT:                  "type": "Identifier",
//CHECK-NEXT:                  "name": "a"
//CHECK-NEXT:                }
//CHECK-NEXT:              ],
//CHECK-NEXT:              "body": {
//CHECK-NEXT:                "type": "BlockStatement",
//CHECK-NEXT:                "body": [
//CHECK-NEXT:                  {
//CHECK-NEXT:                    "type": "ReturnStatement",
//CHECK-NEXT:                    "argument": {
//CHECK-NEXT:                      "type": "NumericLiteral",
//CHECK-NEXT:                      "value": 20,
//CHECK-NEXT:                      "raw": "20"
//CHECK-NEXT:                    }
//CHECK-NEXT:                  }
//CHECK-NEXT:                ]
//CHECK-NEXT:              },
//CHECK-NEXT:              "expression": false,
//CHECK-NEXT:              "async": false
//CHECK-NEXT:            },
//CHECK-NEXT:            "id": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "t3"
//CHECK-NEXT:            }
//CHECK-NEXT:          }
//CHECK-NEXT:        ]
//CHECK-NEXT:      },

var t4 = (a,b,c) => a;
//CHECK-NEXT:      {
//CHECK-NEXT:        "type": "VariableDeclaration",
//CHECK-NEXT:        "kind": "var",
//CHECK-NEXT:        "declarations": [
//CHECK-NEXT:          {
//CHECK-NEXT:            "type": "VariableDeclarator",
//CHECK-NEXT:            "init": {
//CHECK-NEXT:              "type": "ArrowFunctionExpression",
//CHECK-NEXT:              "id": null,
//CHECK-NEXT:              "params": [
//CHECK-NEXT:                {
//CHECK-NEXT:                  "type": "Identifier",
//CHECK-NEXT:                  "name": "a"
//CHECK-NEXT:                },
//CHECK-NEXT:                {
//CHECK-NEXT:                  "type": "Identifier",
//CHECK-NEXT:                  "name": "b"
//CHECK-NEXT:                },
//CHECK-NEXT:                {
//CHECK-NEXT:                  "type": "Identifier",
//CHECK-NEXT:                  "name": "c"
//CHECK-NEXT:                }
//CHECK-NEXT:              ],
//CHECK-NEXT:              "body": {
//CHECK-NEXT:                "type": "Identifier",
//CHECK-NEXT:                "name": "a"
//CHECK-NEXT:              },
//CHECK-NEXT:              "expression": true,
//CHECK-NEXT:              "async": false
//CHECK-NEXT:            },
//CHECK-NEXT:            "id": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "t4"
//CHECK-NEXT:            }
//CHECK-NEXT:          }
//CHECK-NEXT:        ]
//CHECK-NEXT:      },

var t5 = () => 3;
//CHECK-NEXT:      {
//CHECK-NEXT:        "type": "VariableDeclaration",
//CHECK-NEXT:        "kind": "var",
//CHECK-NEXT:        "declarations": [
//CHECK-NEXT:          {
//CHECK-NEXT:            "type": "VariableDeclarator",
//CHECK-NEXT:            "init": {
//CHECK-NEXT:              "type": "ArrowFunctionExpression",
//CHECK-NEXT:              "id": null,
//CHECK-NEXT:              "params": [],
//CHECK-NEXT:              "body": {
//CHECK-NEXT:                "type": "NumericLiteral",
//CHECK-NEXT:                "value": 3,
//CHECK-NEXT:                "raw": "3"
//CHECK-NEXT:              },
//CHECK-NEXT:              "expression": true,
//CHECK-NEXT:              "async": false
//CHECK-NEXT:            },
//CHECK-NEXT:            "id": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "t5"
//CHECK-NEXT:            }
//CHECK-NEXT:          }
//CHECK-NEXT:        ]
//CHECK-NEXT:      },

var t6 = (a,) => 3;
//CHECK-NEXT:      {
//CHECK-NEXT:        "type": "VariableDeclaration",
//CHECK-NEXT:        "kind": "var",
//CHECK-NEXT:        "declarations": [
//CHECK-NEXT:          {
//CHECK-NEXT:            "type": "VariableDeclarator",
//CHECK-NEXT:            "init": {
//CHECK-NEXT:              "type": "ArrowFunctionExpression",
//CHECK-NEXT:              "id": null,
//CHECK-NEXT:              "params": [
//CHECK-NEXT:                {
//CHECK-NEXT:                  "type": "Identifier",
//CHECK-NEXT:                  "name": "a"
//CHECK-NEXT:                }
//CHECK-NEXT:              ],
//CHECK-NEXT:              "body": {
//CHECK-NEXT:                "type": "NumericLiteral",
//CHECK-NEXT:                "value": 3,
//CHECK-NEXT:                "raw": "3"
//CHECK-NEXT:              },
//CHECK-NEXT:              "expression": true,
//CHECK-NEXT:              "async": false
//CHECK-NEXT:            },
//CHECK-NEXT:            "id": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "t6"
//CHECK-NEXT:            }
//CHECK-NEXT:          }
//CHECK-NEXT:        ]
//CHECK-NEXT:      }

//CHECK-NEXT:    ]
//CHECK-NEXT:  }
