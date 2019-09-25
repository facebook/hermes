// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

function *foo() {
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo",
// CHECK-NEXT:         "typeAnnotation": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": null,
// CHECK-NEXT:       "generator": true
// CHECK-NEXT:     },

(function* bar() {
});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "FunctionExpression",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "bar",
// CHECK-NEXT:           "typeAnnotation": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         },
// CHECK-NEXT:         "generator": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(function *() {
  yield;
  yield
  1;
  yield 1;
  yield* 1;
});
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "FunctionExpression",
// CHECK-NEXT:        "id": null,
// CHECK-NEXT:        "params": [],
// CHECK-NEXT:        "body": {
// CHECK-NEXT:          "type": "BlockStatement",
// CHECK-NEXT:          "body": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "ExpressionStatement",
// CHECK-NEXT:              "expression": {
// CHECK-NEXT:                "type": "YieldExpression",
// CHECK-NEXT:                "argument": null,
// CHECK-NEXT:                "delegate": false
// CHECK-NEXT:              },
// CHECK-NEXT:              "directive": null
// CHECK-NEXT:            },
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "ExpressionStatement",
// CHECK-NEXT:              "expression": {
// CHECK-NEXT:                "type": "YieldExpression",
// CHECK-NEXT:                "argument": null,
// CHECK-NEXT:                "delegate": false
// CHECK-NEXT:              },
// CHECK-NEXT:              "directive": null
// CHECK-NEXT:            },
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "ExpressionStatement",
// CHECK-NEXT:              "expression": {
// CHECK-NEXT:                "type": "NumericLiteral",
// CHECK-NEXT:                "value": 1,
// CHECK-NEXT:                "raw": "1"
// CHECK-NEXT:              },
// CHECK-NEXT:              "directive": null
// CHECK-NEXT:            },
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "ExpressionStatement",
// CHECK-NEXT:              "expression": {
// CHECK-NEXT:                "type": "YieldExpression",
// CHECK-NEXT:                "argument": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "delegate": false
// CHECK-NEXT:              },
// CHECK-NEXT:              "directive": null
// CHECK-NEXT:            },
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "ExpressionStatement",
// CHECK-NEXT:              "expression": {
// CHECK-NEXT:                "type": "YieldExpression",
// CHECK-NEXT:                "argument": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "delegate": true
// CHECK-NEXT:              },
// CHECK-NEXT:              "directive": null
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        },
// CHECK-NEXT:        "generator": true
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },

function *f1() {
  function f2() {
    var yield = 3;
    function *f3() {
    }
  }
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "FunctionDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "f1",
// CHECK-NEXT:        "typeAnnotation": null
// CHECK-NEXT:      },
// CHECK-NEXT:      "params": [],
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "BlockStatement",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "FunctionDeclaration",
// CHECK-NEXT:            "id": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "f2",
// CHECK-NEXT:              "typeAnnotation": null
// CHECK-NEXT:            },
// CHECK-NEXT:            "params": [],
// CHECK-NEXT:            "body": {
// CHECK-NEXT:              "type": "BlockStatement",
// CHECK-NEXT:              "body": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "VariableDeclaration",
// CHECK-NEXT:                  "kind": "var",
// CHECK-NEXT:                  "declarations": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "VariableDeclarator",
// CHECK-NEXT:                      "init": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 3,
// CHECK-NEXT:                        "raw": "3"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "id": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "yield",
// CHECK-NEXT:                        "typeAnnotation": null
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "FunctionDeclaration",
// CHECK-NEXT:                  "id": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "f3",
// CHECK-NEXT:                    "typeAnnotation": null
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "params": [],
// CHECK-NEXT:                  "body": {
// CHECK-NEXT:                    "type": "BlockStatement",
// CHECK-NEXT:                    "body": []
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "returnType": null,
// CHECK-NEXT:                  "generator": true
// CHECK-NEXT:                }
// CHECK-NEXT:              ]
// CHECK-NEXT:            },
// CHECK-NEXT:            "returnType": null,
// CHECK-NEXT:            "generator": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      },
// CHECK-NEXT:      "returnType": null,
// CHECK-NEXT:      "generator": true
// CHECK-NEXT:    },

function *yieldInExpr() {
  x ? yield : y;
  (yield);
  [yield];
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "yieldInExpr",
// CHECK-NEXT:         "typeAnnotation": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExpressionStatement",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "ConditionalExpression",
// CHECK-NEXT:               "test": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "x",
// CHECK-NEXT:                 "typeAnnotation": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "alternate": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "y",
// CHECK-NEXT:                 "typeAnnotation": null
// CHECK-NEXT:               },
// CHECK-NEXT:               "consequent": {
// CHECK-NEXT:                 "type": "YieldExpression",
// CHECK-NEXT:                 "argument": null,
// CHECK-NEXT:                 "delegate": false
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "directive": null
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExpressionStatement",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "YieldExpression",
// CHECK-NEXT:               "argument": null,
// CHECK-NEXT:               "delegate": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "directive": null
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExpressionStatement",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "ArrayExpression",
// CHECK-NEXT:               "elements": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "YieldExpression",
// CHECK-NEXT:                   "argument": null,
// CHECK-NEXT:                   "delegate": false
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "trailingComma": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "directive": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": null,
// CHECK-NEXT:       "generator": true
// CHECK-NEXT:     },

function *hasYieldInside(){ var y = function yield(){}; }
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "hasYieldInside",
// CHECK-NEXT:         "typeAnnotation": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "VariableDeclaration",
// CHECK-NEXT:             "kind": "var",
// CHECK-NEXT:             "declarations": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "VariableDeclarator",
// CHECK-NEXT:                 "init": {
// CHECK-NEXT:                   "type": "FunctionExpression",
// CHECK-NEXT:                   "id": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "yield",
// CHECK-NEXT:                     "typeAnnotation": null
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": []
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "generator": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "y",
// CHECK-NEXT:                   "typeAnnotation": null
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": null,
// CHECK-NEXT:       "generator": true
// CHECK-NEXT:     },

function *yield() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "yield",
// CHECK-NEXT:         "typeAnnotation": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": null,
// CHECK-NEXT:       "generator": true
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
