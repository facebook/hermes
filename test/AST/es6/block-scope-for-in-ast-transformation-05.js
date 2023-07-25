/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc %s -bs -dump-ast | %FileCheckOrRegen %s --match-full-lines --check-prefix=PRE
// RUN: %hermesc %s -bs -dump-transformed-ast | %FileCheckOrRegen %s --match-full-lines --check-prefix=POST

function foo() {}
function bar() {}

for (let [a,b] in foo) {
    bar(a,b);
}

// Auto-generated content below. Please do not modify manually.

// PRE:{
// PRE-NEXT:  "type": "Program",
// PRE-NEXT:  "body": [
// PRE-NEXT:    {
// PRE-NEXT:      "type": "FunctionDeclaration",
// PRE-NEXT:      "id": {
// PRE-NEXT:        "type": "Identifier",
// PRE-NEXT:        "name": "foo"
// PRE-NEXT:      },
// PRE-NEXT:      "params": [],
// PRE-NEXT:      "body": {
// PRE-NEXT:        "type": "BlockStatement",
// PRE-NEXT:        "body": []
// PRE-NEXT:      },
// PRE-NEXT:      "generator": false,
// PRE-NEXT:      "async": false
// PRE-NEXT:    },
// PRE-NEXT:    {
// PRE-NEXT:      "type": "FunctionDeclaration",
// PRE-NEXT:      "id": {
// PRE-NEXT:        "type": "Identifier",
// PRE-NEXT:        "name": "bar"
// PRE-NEXT:      },
// PRE-NEXT:      "params": [],
// PRE-NEXT:      "body": {
// PRE-NEXT:        "type": "BlockStatement",
// PRE-NEXT:        "body": []
// PRE-NEXT:      },
// PRE-NEXT:      "generator": false,
// PRE-NEXT:      "async": false
// PRE-NEXT:    },
// PRE-NEXT:    {
// PRE-NEXT:      "type": "ForInStatement",
// PRE-NEXT:      "left": {
// PRE-NEXT:        "type": "VariableDeclaration",
// PRE-NEXT:        "kind": "let",
// PRE-NEXT:        "declarations": [
// PRE-NEXT:          {
// PRE-NEXT:            "type": "VariableDeclarator",
// PRE-NEXT:            "init": null,
// PRE-NEXT:            "id": {
// PRE-NEXT:              "type": "ArrayPattern",
// PRE-NEXT:              "elements": [
// PRE-NEXT:                {
// PRE-NEXT:                  "type": "Identifier",
// PRE-NEXT:                  "name": "a"
// PRE-NEXT:                },
// PRE-NEXT:                {
// PRE-NEXT:                  "type": "Identifier",
// PRE-NEXT:                  "name": "b"
// PRE-NEXT:                }
// PRE-NEXT:              ]
// PRE-NEXT:            }
// PRE-NEXT:          }
// PRE-NEXT:        ]
// PRE-NEXT:      },
// PRE-NEXT:      "right": {
// PRE-NEXT:        "type": "Identifier",
// PRE-NEXT:        "name": "foo"
// PRE-NEXT:      },
// PRE-NEXT:      "body": {
// PRE-NEXT:        "type": "BlockStatement",
// PRE-NEXT:        "body": [
// PRE-NEXT:          {
// PRE-NEXT:            "type": "ExpressionStatement",
// PRE-NEXT:            "expression": {
// PRE-NEXT:              "type": "CallExpression",
// PRE-NEXT:              "callee": {
// PRE-NEXT:                "type": "Identifier",
// PRE-NEXT:                "name": "bar"
// PRE-NEXT:              },
// PRE-NEXT:              "arguments": [
// PRE-NEXT:                {
// PRE-NEXT:                  "type": "Identifier",
// PRE-NEXT:                  "name": "a"
// PRE-NEXT:                },
// PRE-NEXT:                {
// PRE-NEXT:                  "type": "Identifier",
// PRE-NEXT:                  "name": "b"
// PRE-NEXT:                }
// PRE-NEXT:              ]
// PRE-NEXT:            },
// PRE-NEXT:            "directive": null
// PRE-NEXT:          }
// PRE-NEXT:        ]
// PRE-NEXT:      }
// PRE-NEXT:    }
// PRE-NEXT:  ]
// PRE-NEXT:}

// POST:{
// POST-NEXT:  "type": "Program",
// POST-NEXT:  "body": [
// POST-NEXT:    {
// POST-NEXT:      "type": "FunctionDeclaration",
// POST-NEXT:      "id": {
// POST-NEXT:        "type": "Identifier",
// POST-NEXT:        "name": "foo"
// POST-NEXT:      },
// POST-NEXT:      "params": [],
// POST-NEXT:      "body": {
// POST-NEXT:        "type": "BlockStatement",
// POST-NEXT:        "body": []
// POST-NEXT:      },
// POST-NEXT:      "generator": false,
// POST-NEXT:      "async": false
// POST-NEXT:    },
// POST-NEXT:    {
// POST-NEXT:      "type": "FunctionDeclaration",
// POST-NEXT:      "id": {
// POST-NEXT:        "type": "Identifier",
// POST-NEXT:        "name": "bar"
// POST-NEXT:      },
// POST-NEXT:      "params": [],
// POST-NEXT:      "body": {
// POST-NEXT:        "type": "BlockStatement",
// POST-NEXT:        "body": []
// POST-NEXT:      },
// POST-NEXT:      "generator": false,
// POST-NEXT:      "async": false
// POST-NEXT:    },
// POST-NEXT:    {
// POST-NEXT:      "type": "LabeledStatement",
// POST-NEXT:      "label": {
// POST-NEXT:        "type": "Identifier",
// POST-NEXT:        "name": "?anon_1_wrapperBlock"
// POST-NEXT:      },
// POST-NEXT:      "body": {
// POST-NEXT:        "type": "BlockStatement",
// POST-NEXT:        "body": [
// POST-NEXT:          {
// POST-NEXT:            "type": "VariableDeclaration",
// POST-NEXT:            "kind": "let",
// POST-NEXT:            "declarations": [
// POST-NEXT:              {
// POST-NEXT:                "type": "VariableDeclarator",
// POST-NEXT:                "init": null,
// POST-NEXT:                "id": {
// POST-NEXT:                  "type": "Identifier",
// POST-NEXT:                  "name": "?anon_0_forInOf"
// POST-NEXT:                }
// POST-NEXT:              }
// POST-NEXT:            ]
// POST-NEXT:          },
// POST-NEXT:          {
// POST-NEXT:            "type": "ForInStatement",
// POST-NEXT:            "left": {
// POST-NEXT:              "type": "Identifier",
// POST-NEXT:              "name": "?anon_0_forInOf"
// POST-NEXT:            },
// POST-NEXT:            "right": {
// POST-NEXT:              "type": "Identifier",
// POST-NEXT:              "name": "foo"
// POST-NEXT:            },
// POST-NEXT:            "body": {
// POST-NEXT:              "type": "BlockStatement",
// POST-NEXT:              "body": [
// POST-NEXT:                {
// POST-NEXT:                  "type": "VariableDeclaration",
// POST-NEXT:                  "kind": "let",
// POST-NEXT:                  "declarations": [
// POST-NEXT:                    {
// POST-NEXT:                      "type": "VariableDeclarator",
// POST-NEXT:                      "init": {
// POST-NEXT:                        "type": "Identifier",
// POST-NEXT:                        "name": "?anon_0_forInOf"
// POST-NEXT:                      },
// POST-NEXT:                      "id": {
// POST-NEXT:                        "type": "ArrayPattern",
// POST-NEXT:                        "elements": [
// POST-NEXT:                          {
// POST-NEXT:                            "type": "Identifier",
// POST-NEXT:                            "name": "a"
// POST-NEXT:                          },
// POST-NEXT:                          {
// POST-NEXT:                            "type": "Identifier",
// POST-NEXT:                            "name": "b"
// POST-NEXT:                          }
// POST-NEXT:                        ]
// POST-NEXT:                      }
// POST-NEXT:                    }
// POST-NEXT:                  ]
// POST-NEXT:                },
// POST-NEXT:                {
// POST-NEXT:                  "type": "BlockStatement",
// POST-NEXT:                  "body": [
// POST-NEXT:                    {
// POST-NEXT:                      "type": "ExpressionStatement",
// POST-NEXT:                      "expression": {
// POST-NEXT:                        "type": "CallExpression",
// POST-NEXT:                        "callee": {
// POST-NEXT:                          "type": "Identifier",
// POST-NEXT:                          "name": "bar"
// POST-NEXT:                        },
// POST-NEXT:                        "arguments": [
// POST-NEXT:                          {
// POST-NEXT:                            "type": "Identifier",
// POST-NEXT:                            "name": "a"
// POST-NEXT:                          },
// POST-NEXT:                          {
// POST-NEXT:                            "type": "Identifier",
// POST-NEXT:                            "name": "b"
// POST-NEXT:                          }
// POST-NEXT:                        ]
// POST-NEXT:                      },
// POST-NEXT:                      "directive": null
// POST-NEXT:                    }
// POST-NEXT:                  ]
// POST-NEXT:                }
// POST-NEXT:              ]
// POST-NEXT:            }
// POST-NEXT:          },
// POST-NEXT:          {
// POST-NEXT:            "type": "BreakStatement",
// POST-NEXT:            "label": {
// POST-NEXT:              "type": "Identifier",
// POST-NEXT:              "name": "?anon_1_wrapperBlock"
// POST-NEXT:            }
// POST-NEXT:          },
// POST-NEXT:          {
// POST-NEXT:            "type": "VariableDeclaration",
// POST-NEXT:            "kind": "let",
// POST-NEXT:            "declarations": [
// POST-NEXT:              {
// POST-NEXT:                "type": "VariableDeclarator",
// POST-NEXT:                "init": null,
// POST-NEXT:                "id": {
// POST-NEXT:                  "type": "Identifier",
// POST-NEXT:                  "name": "a"
// POST-NEXT:                }
// POST-NEXT:              },
// POST-NEXT:              {
// POST-NEXT:                "type": "VariableDeclarator",
// POST-NEXT:                "init": null,
// POST-NEXT:                "id": {
// POST-NEXT:                  "type": "Identifier",
// POST-NEXT:                  "name": "b"
// POST-NEXT:                }
// POST-NEXT:              }
// POST-NEXT:            ]
// POST-NEXT:          }
// POST-NEXT:        ]
// POST-NEXT:      }
// POST-NEXT:    }
// POST-NEXT:  ]
// POST-NEXT:}
