/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty=0 %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -dump-ast -pretty %s | %FileCheckOrRegen --match-full-lines %s --check-prefix=CHECK-PRETTY
// RUN: %hermesc -dump-ast -dump-source-location=both -pretty %s | %FileCheckOrRegen --match-full-lines %s --check-prefix=CHECK-SOURCE-LOC
// RUN: %hermesc -dump-ast -Xinclude-empty-ast-nodes -pretty-json %s | %FileCheckOrRegen --match-full-lines %s --check-prefix=CHECK-FULL

function foo() {
  return Math.random();
}

switch (foo()) {
  case 3:
    print('fizz');
    break;
  case 5:
    print('buzz');
    break;
  default:
    print(foo());
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{"type":"Program","body":[{"type":"FunctionDeclaration","id":{"type":"Identifier","name":"foo"},"params":[],"body":{"type":"BlockStatement","body":[{"type":"ReturnStatement","argument":{"type":"CallExpression","callee":{"type":"MemberExpression","object":{"type":"Identifier","name":"Math"},"property":{"type":"Identifier","name":"random"},"computed":false},"arguments":[]}}]},"generator":false,"async":false},{"type":"SwitchStatement","discriminant":{"type":"CallExpression","callee":{"type":"Identifier","name":"foo"},"arguments":[]},"cases":[{"type":"SwitchCase","test":{"type":"NumericLiteral","value":3,"raw":"3"},"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print"},"arguments":[{"type":"StringLiteral","value":"fizz"}]},"directive":null},{"type":"BreakStatement","label":null}]},{"type":"SwitchCase","test":{"type":"NumericLiteral","value":5,"raw":"5"},"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print"},"arguments":[{"type":"StringLiteral","value":"buzz"}]},"directive":null},{"type":"BreakStatement","label":null}]},{"type":"SwitchCase","test":null,"consequent":[{"type":"ExpressionStatement","expression":{"type":"CallExpression","callee":{"type":"Identifier","name":"print"},"arguments":[{"type":"CallExpression","callee":{"type":"Identifier","name":"foo"},"arguments":[]}]},"directive":null}]}]}]}

// CHECK-PRETTY:{
// CHECK-PRETTY-NEXT:  "type": "Program",
// CHECK-PRETTY-NEXT:  "body": [
// CHECK-PRETTY-NEXT:    {
// CHECK-PRETTY-NEXT:      "type": "FunctionDeclaration",
// CHECK-PRETTY-NEXT:      "id": {
// CHECK-PRETTY-NEXT:        "type": "Identifier",
// CHECK-PRETTY-NEXT:        "name": "foo"
// CHECK-PRETTY-NEXT:      },
// CHECK-PRETTY-NEXT:      "params": [],
// CHECK-PRETTY-NEXT:      "body": {
// CHECK-PRETTY-NEXT:        "type": "BlockStatement",
// CHECK-PRETTY-NEXT:        "body": [
// CHECK-PRETTY-NEXT:          {
// CHECK-PRETTY-NEXT:            "type": "ReturnStatement",
// CHECK-PRETTY-NEXT:            "argument": {
// CHECK-PRETTY-NEXT:              "type": "CallExpression",
// CHECK-PRETTY-NEXT:              "callee": {
// CHECK-PRETTY-NEXT:                "type": "MemberExpression",
// CHECK-PRETTY-NEXT:                "object": {
// CHECK-PRETTY-NEXT:                  "type": "Identifier",
// CHECK-PRETTY-NEXT:                  "name": "Math"
// CHECK-PRETTY-NEXT:                },
// CHECK-PRETTY-NEXT:                "property": {
// CHECK-PRETTY-NEXT:                  "type": "Identifier",
// CHECK-PRETTY-NEXT:                  "name": "random"
// CHECK-PRETTY-NEXT:                },
// CHECK-PRETTY-NEXT:                "computed": false
// CHECK-PRETTY-NEXT:              },
// CHECK-PRETTY-NEXT:              "arguments": []
// CHECK-PRETTY-NEXT:            }
// CHECK-PRETTY-NEXT:          }
// CHECK-PRETTY-NEXT:        ]
// CHECK-PRETTY-NEXT:      },
// CHECK-PRETTY-NEXT:      "generator": false,
// CHECK-PRETTY-NEXT:      "async": false
// CHECK-PRETTY-NEXT:    },
// CHECK-PRETTY-NEXT:    {
// CHECK-PRETTY-NEXT:      "type": "SwitchStatement",
// CHECK-PRETTY-NEXT:      "discriminant": {
// CHECK-PRETTY-NEXT:        "type": "CallExpression",
// CHECK-PRETTY-NEXT:        "callee": {
// CHECK-PRETTY-NEXT:          "type": "Identifier",
// CHECK-PRETTY-NEXT:          "name": "foo"
// CHECK-PRETTY-NEXT:        },
// CHECK-PRETTY-NEXT:        "arguments": []
// CHECK-PRETTY-NEXT:      },
// CHECK-PRETTY-NEXT:      "cases": [
// CHECK-PRETTY-NEXT:        {
// CHECK-PRETTY-NEXT:          "type": "SwitchCase",
// CHECK-PRETTY-NEXT:          "test": {
// CHECK-PRETTY-NEXT:            "type": "NumericLiteral",
// CHECK-PRETTY-NEXT:            "value": 3,
// CHECK-PRETTY-NEXT:            "raw": "3"
// CHECK-PRETTY-NEXT:          },
// CHECK-PRETTY-NEXT:          "consequent": [
// CHECK-PRETTY-NEXT:            {
// CHECK-PRETTY-NEXT:              "type": "ExpressionStatement",
// CHECK-PRETTY-NEXT:              "expression": {
// CHECK-PRETTY-NEXT:                "type": "CallExpression",
// CHECK-PRETTY-NEXT:                "callee": {
// CHECK-PRETTY-NEXT:                  "type": "Identifier",
// CHECK-PRETTY-NEXT:                  "name": "print"
// CHECK-PRETTY-NEXT:                },
// CHECK-PRETTY-NEXT:                "arguments": [
// CHECK-PRETTY-NEXT:                  {
// CHECK-PRETTY-NEXT:                    "type": "StringLiteral",
// CHECK-PRETTY-NEXT:                    "value": "fizz"
// CHECK-PRETTY-NEXT:                  }
// CHECK-PRETTY-NEXT:                ]
// CHECK-PRETTY-NEXT:              },
// CHECK-PRETTY-NEXT:              "directive": null
// CHECK-PRETTY-NEXT:            },
// CHECK-PRETTY-NEXT:            {
// CHECK-PRETTY-NEXT:              "type": "BreakStatement",
// CHECK-PRETTY-NEXT:              "label": null
// CHECK-PRETTY-NEXT:            }
// CHECK-PRETTY-NEXT:          ]
// CHECK-PRETTY-NEXT:        },
// CHECK-PRETTY-NEXT:        {
// CHECK-PRETTY-NEXT:          "type": "SwitchCase",
// CHECK-PRETTY-NEXT:          "test": {
// CHECK-PRETTY-NEXT:            "type": "NumericLiteral",
// CHECK-PRETTY-NEXT:            "value": 5,
// CHECK-PRETTY-NEXT:            "raw": "5"
// CHECK-PRETTY-NEXT:          },
// CHECK-PRETTY-NEXT:          "consequent": [
// CHECK-PRETTY-NEXT:            {
// CHECK-PRETTY-NEXT:              "type": "ExpressionStatement",
// CHECK-PRETTY-NEXT:              "expression": {
// CHECK-PRETTY-NEXT:                "type": "CallExpression",
// CHECK-PRETTY-NEXT:                "callee": {
// CHECK-PRETTY-NEXT:                  "type": "Identifier",
// CHECK-PRETTY-NEXT:                  "name": "print"
// CHECK-PRETTY-NEXT:                },
// CHECK-PRETTY-NEXT:                "arguments": [
// CHECK-PRETTY-NEXT:                  {
// CHECK-PRETTY-NEXT:                    "type": "StringLiteral",
// CHECK-PRETTY-NEXT:                    "value": "buzz"
// CHECK-PRETTY-NEXT:                  }
// CHECK-PRETTY-NEXT:                ]
// CHECK-PRETTY-NEXT:              },
// CHECK-PRETTY-NEXT:              "directive": null
// CHECK-PRETTY-NEXT:            },
// CHECK-PRETTY-NEXT:            {
// CHECK-PRETTY-NEXT:              "type": "BreakStatement",
// CHECK-PRETTY-NEXT:              "label": null
// CHECK-PRETTY-NEXT:            }
// CHECK-PRETTY-NEXT:          ]
// CHECK-PRETTY-NEXT:        },
// CHECK-PRETTY-NEXT:        {
// CHECK-PRETTY-NEXT:          "type": "SwitchCase",
// CHECK-PRETTY-NEXT:          "test": null,
// CHECK-PRETTY-NEXT:          "consequent": [
// CHECK-PRETTY-NEXT:            {
// CHECK-PRETTY-NEXT:              "type": "ExpressionStatement",
// CHECK-PRETTY-NEXT:              "expression": {
// CHECK-PRETTY-NEXT:                "type": "CallExpression",
// CHECK-PRETTY-NEXT:                "callee": {
// CHECK-PRETTY-NEXT:                  "type": "Identifier",
// CHECK-PRETTY-NEXT:                  "name": "print"
// CHECK-PRETTY-NEXT:                },
// CHECK-PRETTY-NEXT:                "arguments": [
// CHECK-PRETTY-NEXT:                  {
// CHECK-PRETTY-NEXT:                    "type": "CallExpression",
// CHECK-PRETTY-NEXT:                    "callee": {
// CHECK-PRETTY-NEXT:                      "type": "Identifier",
// CHECK-PRETTY-NEXT:                      "name": "foo"
// CHECK-PRETTY-NEXT:                    },
// CHECK-PRETTY-NEXT:                    "arguments": []
// CHECK-PRETTY-NEXT:                  }
// CHECK-PRETTY-NEXT:                ]
// CHECK-PRETTY-NEXT:              },
// CHECK-PRETTY-NEXT:              "directive": null
// CHECK-PRETTY-NEXT:            }
// CHECK-PRETTY-NEXT:          ]
// CHECK-PRETTY-NEXT:        }
// CHECK-PRETTY-NEXT:      ]
// CHECK-PRETTY-NEXT:    }
// CHECK-PRETTY-NEXT:  ]
// CHECK-PRETTY-NEXT:}

// CHECK-SOURCE-LOC:{
// CHECK-SOURCE-LOC-NEXT:  "type": "Program",
// CHECK-SOURCE-LOC-NEXT:  "body": [
// CHECK-SOURCE-LOC-NEXT:    {
// CHECK-SOURCE-LOC-NEXT:      "type": "FunctionDeclaration",
// CHECK-SOURCE-LOC-NEXT:      "id": {
// CHECK-SOURCE-LOC-NEXT:        "type": "Identifier",
// CHECK-SOURCE-LOC-NEXT:        "name": "foo",
// CHECK-SOURCE-LOC-NEXT:        "loc": {
// CHECK-SOURCE-LOC-NEXT:          "start": {
// CHECK-SOURCE-LOC-NEXT:            "line": 13,
// CHECK-SOURCE-LOC-NEXT:            "column": 10
// CHECK-SOURCE-LOC-NEXT:          },
// CHECK-SOURCE-LOC-NEXT:          "end": {
// CHECK-SOURCE-LOC-NEXT:            "line": 13,
// CHECK-SOURCE-LOC-NEXT:            "column": 13
// CHECK-SOURCE-LOC-NEXT:          }
// CHECK-SOURCE-LOC-NEXT:        },
// CHECK-SOURCE-LOC-NEXT:        "range": [
// CHECK-SOURCE-LOC-NEXT:          668,
// CHECK-SOURCE-LOC-NEXT:          671
// CHECK-SOURCE-LOC-NEXT:        ]
// CHECK-SOURCE-LOC-NEXT:      },
// CHECK-SOURCE-LOC-NEXT:      "params": [],
// CHECK-SOURCE-LOC-NEXT:      "body": {
// CHECK-SOURCE-LOC-NEXT:        "type": "BlockStatement",
// CHECK-SOURCE-LOC-NEXT:        "body": [
// CHECK-SOURCE-LOC-NEXT:          {
// CHECK-SOURCE-LOC-NEXT:            "type": "ReturnStatement",
// CHECK-SOURCE-LOC-NEXT:            "argument": {
// CHECK-SOURCE-LOC-NEXT:              "type": "CallExpression",
// CHECK-SOURCE-LOC-NEXT:              "callee": {
// CHECK-SOURCE-LOC-NEXT:                "type": "MemberExpression",
// CHECK-SOURCE-LOC-NEXT:                "object": {
// CHECK-SOURCE-LOC-NEXT:                  "type": "Identifier",
// CHECK-SOURCE-LOC-NEXT:                  "name": "Math",
// CHECK-SOURCE-LOC-NEXT:                  "loc": {
// CHECK-SOURCE-LOC-NEXT:                    "start": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 14,
// CHECK-SOURCE-LOC-NEXT:                      "column": 10
// CHECK-SOURCE-LOC-NEXT:                    },
// CHECK-SOURCE-LOC-NEXT:                    "end": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 14,
// CHECK-SOURCE-LOC-NEXT:                      "column": 14
// CHECK-SOURCE-LOC-NEXT:                    }
// CHECK-SOURCE-LOC-NEXT:                  },
// CHECK-SOURCE-LOC-NEXT:                  "range": [
// CHECK-SOURCE-LOC-NEXT:                    685,
// CHECK-SOURCE-LOC-NEXT:                    689
// CHECK-SOURCE-LOC-NEXT:                  ]
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "property": {
// CHECK-SOURCE-LOC-NEXT:                  "type": "Identifier",
// CHECK-SOURCE-LOC-NEXT:                  "name": "random",
// CHECK-SOURCE-LOC-NEXT:                  "loc": {
// CHECK-SOURCE-LOC-NEXT:                    "start": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 14,
// CHECK-SOURCE-LOC-NEXT:                      "column": 15
// CHECK-SOURCE-LOC-NEXT:                    },
// CHECK-SOURCE-LOC-NEXT:                    "end": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 14,
// CHECK-SOURCE-LOC-NEXT:                      "column": 21
// CHECK-SOURCE-LOC-NEXT:                    }
// CHECK-SOURCE-LOC-NEXT:                  },
// CHECK-SOURCE-LOC-NEXT:                  "range": [
// CHECK-SOURCE-LOC-NEXT:                    690,
// CHECK-SOURCE-LOC-NEXT:                    696
// CHECK-SOURCE-LOC-NEXT:                  ]
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "computed": false,
// CHECK-SOURCE-LOC-NEXT:                "loc": {
// CHECK-SOURCE-LOC-NEXT:                  "start": {
// CHECK-SOURCE-LOC-NEXT:                    "line": 14,
// CHECK-SOURCE-LOC-NEXT:                    "column": 10
// CHECK-SOURCE-LOC-NEXT:                  },
// CHECK-SOURCE-LOC-NEXT:                  "end": {
// CHECK-SOURCE-LOC-NEXT:                    "line": 14,
// CHECK-SOURCE-LOC-NEXT:                    "column": 21
// CHECK-SOURCE-LOC-NEXT:                  }
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "range": [
// CHECK-SOURCE-LOC-NEXT:                  685,
// CHECK-SOURCE-LOC-NEXT:                  696
// CHECK-SOURCE-LOC-NEXT:                ]
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "arguments": [],
// CHECK-SOURCE-LOC-NEXT:              "loc": {
// CHECK-SOURCE-LOC-NEXT:                "start": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 14,
// CHECK-SOURCE-LOC-NEXT:                  "column": 10
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "end": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 14,
// CHECK-SOURCE-LOC-NEXT:                  "column": 23
// CHECK-SOURCE-LOC-NEXT:                }
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "range": [
// CHECK-SOURCE-LOC-NEXT:                685,
// CHECK-SOURCE-LOC-NEXT:                698
// CHECK-SOURCE-LOC-NEXT:              ]
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            "loc": {
// CHECK-SOURCE-LOC-NEXT:              "start": {
// CHECK-SOURCE-LOC-NEXT:                "line": 14,
// CHECK-SOURCE-LOC-NEXT:                "column": 3
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "end": {
// CHECK-SOURCE-LOC-NEXT:                "line": 14,
// CHECK-SOURCE-LOC-NEXT:                "column": 24
// CHECK-SOURCE-LOC-NEXT:              }
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            "range": [
// CHECK-SOURCE-LOC-NEXT:              678,
// CHECK-SOURCE-LOC-NEXT:              699
// CHECK-SOURCE-LOC-NEXT:            ]
// CHECK-SOURCE-LOC-NEXT:          }
// CHECK-SOURCE-LOC-NEXT:        ],
// CHECK-SOURCE-LOC-NEXT:        "loc": {
// CHECK-SOURCE-LOC-NEXT:          "start": {
// CHECK-SOURCE-LOC-NEXT:            "line": 13,
// CHECK-SOURCE-LOC-NEXT:            "column": 16
// CHECK-SOURCE-LOC-NEXT:          },
// CHECK-SOURCE-LOC-NEXT:          "end": {
// CHECK-SOURCE-LOC-NEXT:            "line": 15,
// CHECK-SOURCE-LOC-NEXT:            "column": 2
// CHECK-SOURCE-LOC-NEXT:          }
// CHECK-SOURCE-LOC-NEXT:        },
// CHECK-SOURCE-LOC-NEXT:        "range": [
// CHECK-SOURCE-LOC-NEXT:          674,
// CHECK-SOURCE-LOC-NEXT:          701
// CHECK-SOURCE-LOC-NEXT:        ]
// CHECK-SOURCE-LOC-NEXT:      },
// CHECK-SOURCE-LOC-NEXT:      "generator": false,
// CHECK-SOURCE-LOC-NEXT:      "async": false,
// CHECK-SOURCE-LOC-NEXT:      "loc": {
// CHECK-SOURCE-LOC-NEXT:        "start": {
// CHECK-SOURCE-LOC-NEXT:          "line": 13,
// CHECK-SOURCE-LOC-NEXT:          "column": 1
// CHECK-SOURCE-LOC-NEXT:        },
// CHECK-SOURCE-LOC-NEXT:        "end": {
// CHECK-SOURCE-LOC-NEXT:          "line": 15,
// CHECK-SOURCE-LOC-NEXT:          "column": 2
// CHECK-SOURCE-LOC-NEXT:        }
// CHECK-SOURCE-LOC-NEXT:      },
// CHECK-SOURCE-LOC-NEXT:      "range": [
// CHECK-SOURCE-LOC-NEXT:        659,
// CHECK-SOURCE-LOC-NEXT:        701
// CHECK-SOURCE-LOC-NEXT:      ]
// CHECK-SOURCE-LOC-NEXT:    },
// CHECK-SOURCE-LOC-NEXT:    {
// CHECK-SOURCE-LOC-NEXT:      "type": "SwitchStatement",
// CHECK-SOURCE-LOC-NEXT:      "discriminant": {
// CHECK-SOURCE-LOC-NEXT:        "type": "CallExpression",
// CHECK-SOURCE-LOC-NEXT:        "callee": {
// CHECK-SOURCE-LOC-NEXT:          "type": "Identifier",
// CHECK-SOURCE-LOC-NEXT:          "name": "foo",
// CHECK-SOURCE-LOC-NEXT:          "loc": {
// CHECK-SOURCE-LOC-NEXT:            "start": {
// CHECK-SOURCE-LOC-NEXT:              "line": 17,
// CHECK-SOURCE-LOC-NEXT:              "column": 9
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            "end": {
// CHECK-SOURCE-LOC-NEXT:              "line": 17,
// CHECK-SOURCE-LOC-NEXT:              "column": 12
// CHECK-SOURCE-LOC-NEXT:            }
// CHECK-SOURCE-LOC-NEXT:          },
// CHECK-SOURCE-LOC-NEXT:          "range": [
// CHECK-SOURCE-LOC-NEXT:            711,
// CHECK-SOURCE-LOC-NEXT:            714
// CHECK-SOURCE-LOC-NEXT:          ]
// CHECK-SOURCE-LOC-NEXT:        },
// CHECK-SOURCE-LOC-NEXT:        "arguments": [],
// CHECK-SOURCE-LOC-NEXT:        "loc": {
// CHECK-SOURCE-LOC-NEXT:          "start": {
// CHECK-SOURCE-LOC-NEXT:            "line": 17,
// CHECK-SOURCE-LOC-NEXT:            "column": 9
// CHECK-SOURCE-LOC-NEXT:          },
// CHECK-SOURCE-LOC-NEXT:          "end": {
// CHECK-SOURCE-LOC-NEXT:            "line": 17,
// CHECK-SOURCE-LOC-NEXT:            "column": 14
// CHECK-SOURCE-LOC-NEXT:          }
// CHECK-SOURCE-LOC-NEXT:        },
// CHECK-SOURCE-LOC-NEXT:        "range": [
// CHECK-SOURCE-LOC-NEXT:          711,
// CHECK-SOURCE-LOC-NEXT:          716
// CHECK-SOURCE-LOC-NEXT:        ]
// CHECK-SOURCE-LOC-NEXT:      },
// CHECK-SOURCE-LOC-NEXT:      "cases": [
// CHECK-SOURCE-LOC-NEXT:        {
// CHECK-SOURCE-LOC-NEXT:          "type": "SwitchCase",
// CHECK-SOURCE-LOC-NEXT:          "test": {
// CHECK-SOURCE-LOC-NEXT:            "type": "NumericLiteral",
// CHECK-SOURCE-LOC-NEXT:            "value": 3,
// CHECK-SOURCE-LOC-NEXT:            "raw": "3",
// CHECK-SOURCE-LOC-NEXT:            "loc": {
// CHECK-SOURCE-LOC-NEXT:              "start": {
// CHECK-SOURCE-LOC-NEXT:                "line": 18,
// CHECK-SOURCE-LOC-NEXT:                "column": 8
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "end": {
// CHECK-SOURCE-LOC-NEXT:                "line": 18,
// CHECK-SOURCE-LOC-NEXT:                "column": 9
// CHECK-SOURCE-LOC-NEXT:              }
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            "range": [
// CHECK-SOURCE-LOC-NEXT:              727,
// CHECK-SOURCE-LOC-NEXT:              728
// CHECK-SOURCE-LOC-NEXT:            ]
// CHECK-SOURCE-LOC-NEXT:          },
// CHECK-SOURCE-LOC-NEXT:          "consequent": [
// CHECK-SOURCE-LOC-NEXT:            {
// CHECK-SOURCE-LOC-NEXT:              "type": "ExpressionStatement",
// CHECK-SOURCE-LOC-NEXT:              "expression": {
// CHECK-SOURCE-LOC-NEXT:                "type": "CallExpression",
// CHECK-SOURCE-LOC-NEXT:                "callee": {
// CHECK-SOURCE-LOC-NEXT:                  "type": "Identifier",
// CHECK-SOURCE-LOC-NEXT:                  "name": "print",
// CHECK-SOURCE-LOC-NEXT:                  "loc": {
// CHECK-SOURCE-LOC-NEXT:                    "start": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 19,
// CHECK-SOURCE-LOC-NEXT:                      "column": 5
// CHECK-SOURCE-LOC-NEXT:                    },
// CHECK-SOURCE-LOC-NEXT:                    "end": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 19,
// CHECK-SOURCE-LOC-NEXT:                      "column": 10
// CHECK-SOURCE-LOC-NEXT:                    }
// CHECK-SOURCE-LOC-NEXT:                  },
// CHECK-SOURCE-LOC-NEXT:                  "range": [
// CHECK-SOURCE-LOC-NEXT:                    734,
// CHECK-SOURCE-LOC-NEXT:                    739
// CHECK-SOURCE-LOC-NEXT:                  ]
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "arguments": [
// CHECK-SOURCE-LOC-NEXT:                  {
// CHECK-SOURCE-LOC-NEXT:                    "type": "StringLiteral",
// CHECK-SOURCE-LOC-NEXT:                    "value": "fizz",
// CHECK-SOURCE-LOC-NEXT:                    "loc": {
// CHECK-SOURCE-LOC-NEXT:                      "start": {
// CHECK-SOURCE-LOC-NEXT:                        "line": 19,
// CHECK-SOURCE-LOC-NEXT:                        "column": 11
// CHECK-SOURCE-LOC-NEXT:                      },
// CHECK-SOURCE-LOC-NEXT:                      "end": {
// CHECK-SOURCE-LOC-NEXT:                        "line": 19,
// CHECK-SOURCE-LOC-NEXT:                        "column": 17
// CHECK-SOURCE-LOC-NEXT:                      }
// CHECK-SOURCE-LOC-NEXT:                    },
// CHECK-SOURCE-LOC-NEXT:                    "range": [
// CHECK-SOURCE-LOC-NEXT:                      740,
// CHECK-SOURCE-LOC-NEXT:                      746
// CHECK-SOURCE-LOC-NEXT:                    ]
// CHECK-SOURCE-LOC-NEXT:                  }
// CHECK-SOURCE-LOC-NEXT:                ],
// CHECK-SOURCE-LOC-NEXT:                "loc": {
// CHECK-SOURCE-LOC-NEXT:                  "start": {
// CHECK-SOURCE-LOC-NEXT:                    "line": 19,
// CHECK-SOURCE-LOC-NEXT:                    "column": 5
// CHECK-SOURCE-LOC-NEXT:                  },
// CHECK-SOURCE-LOC-NEXT:                  "end": {
// CHECK-SOURCE-LOC-NEXT:                    "line": 19,
// CHECK-SOURCE-LOC-NEXT:                    "column": 18
// CHECK-SOURCE-LOC-NEXT:                  }
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "range": [
// CHECK-SOURCE-LOC-NEXT:                  734,
// CHECK-SOURCE-LOC-NEXT:                  747
// CHECK-SOURCE-LOC-NEXT:                ]
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "directive": null,
// CHECK-SOURCE-LOC-NEXT:              "loc": {
// CHECK-SOURCE-LOC-NEXT:                "start": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 19,
// CHECK-SOURCE-LOC-NEXT:                  "column": 5
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "end": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 19,
// CHECK-SOURCE-LOC-NEXT:                  "column": 19
// CHECK-SOURCE-LOC-NEXT:                }
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "range": [
// CHECK-SOURCE-LOC-NEXT:                734,
// CHECK-SOURCE-LOC-NEXT:                748
// CHECK-SOURCE-LOC-NEXT:              ]
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            {
// CHECK-SOURCE-LOC-NEXT:              "type": "BreakStatement",
// CHECK-SOURCE-LOC-NEXT:              "label": null,
// CHECK-SOURCE-LOC-NEXT:              "loc": {
// CHECK-SOURCE-LOC-NEXT:                "start": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 20,
// CHECK-SOURCE-LOC-NEXT:                  "column": 5
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "end": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 20,
// CHECK-SOURCE-LOC-NEXT:                  "column": 11
// CHECK-SOURCE-LOC-NEXT:                }
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "range": [
// CHECK-SOURCE-LOC-NEXT:                753,
// CHECK-SOURCE-LOC-NEXT:                759
// CHECK-SOURCE-LOC-NEXT:              ]
// CHECK-SOURCE-LOC-NEXT:            }
// CHECK-SOURCE-LOC-NEXT:          ],
// CHECK-SOURCE-LOC-NEXT:          "loc": {
// CHECK-SOURCE-LOC-NEXT:            "start": {
// CHECK-SOURCE-LOC-NEXT:              "line": 18,
// CHECK-SOURCE-LOC-NEXT:              "column": 3
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            "end": {
// CHECK-SOURCE-LOC-NEXT:              "line": 20,
// CHECK-SOURCE-LOC-NEXT:              "column": 11
// CHECK-SOURCE-LOC-NEXT:            }
// CHECK-SOURCE-LOC-NEXT:          },
// CHECK-SOURCE-LOC-NEXT:          "range": [
// CHECK-SOURCE-LOC-NEXT:            722,
// CHECK-SOURCE-LOC-NEXT:            759
// CHECK-SOURCE-LOC-NEXT:          ]
// CHECK-SOURCE-LOC-NEXT:        },
// CHECK-SOURCE-LOC-NEXT:        {
// CHECK-SOURCE-LOC-NEXT:          "type": "SwitchCase",
// CHECK-SOURCE-LOC-NEXT:          "test": {
// CHECK-SOURCE-LOC-NEXT:            "type": "NumericLiteral",
// CHECK-SOURCE-LOC-NEXT:            "value": 5,
// CHECK-SOURCE-LOC-NEXT:            "raw": "5",
// CHECK-SOURCE-LOC-NEXT:            "loc": {
// CHECK-SOURCE-LOC-NEXT:              "start": {
// CHECK-SOURCE-LOC-NEXT:                "line": 21,
// CHECK-SOURCE-LOC-NEXT:                "column": 8
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "end": {
// CHECK-SOURCE-LOC-NEXT:                "line": 21,
// CHECK-SOURCE-LOC-NEXT:                "column": 9
// CHECK-SOURCE-LOC-NEXT:              }
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            "range": [
// CHECK-SOURCE-LOC-NEXT:              767,
// CHECK-SOURCE-LOC-NEXT:              768
// CHECK-SOURCE-LOC-NEXT:            ]
// CHECK-SOURCE-LOC-NEXT:          },
// CHECK-SOURCE-LOC-NEXT:          "consequent": [
// CHECK-SOURCE-LOC-NEXT:            {
// CHECK-SOURCE-LOC-NEXT:              "type": "ExpressionStatement",
// CHECK-SOURCE-LOC-NEXT:              "expression": {
// CHECK-SOURCE-LOC-NEXT:                "type": "CallExpression",
// CHECK-SOURCE-LOC-NEXT:                "callee": {
// CHECK-SOURCE-LOC-NEXT:                  "type": "Identifier",
// CHECK-SOURCE-LOC-NEXT:                  "name": "print",
// CHECK-SOURCE-LOC-NEXT:                  "loc": {
// CHECK-SOURCE-LOC-NEXT:                    "start": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 22,
// CHECK-SOURCE-LOC-NEXT:                      "column": 5
// CHECK-SOURCE-LOC-NEXT:                    },
// CHECK-SOURCE-LOC-NEXT:                    "end": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 22,
// CHECK-SOURCE-LOC-NEXT:                      "column": 10
// CHECK-SOURCE-LOC-NEXT:                    }
// CHECK-SOURCE-LOC-NEXT:                  },
// CHECK-SOURCE-LOC-NEXT:                  "range": [
// CHECK-SOURCE-LOC-NEXT:                    774,
// CHECK-SOURCE-LOC-NEXT:                    779
// CHECK-SOURCE-LOC-NEXT:                  ]
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "arguments": [
// CHECK-SOURCE-LOC-NEXT:                  {
// CHECK-SOURCE-LOC-NEXT:                    "type": "StringLiteral",
// CHECK-SOURCE-LOC-NEXT:                    "value": "buzz",
// CHECK-SOURCE-LOC-NEXT:                    "loc": {
// CHECK-SOURCE-LOC-NEXT:                      "start": {
// CHECK-SOURCE-LOC-NEXT:                        "line": 22,
// CHECK-SOURCE-LOC-NEXT:                        "column": 11
// CHECK-SOURCE-LOC-NEXT:                      },
// CHECK-SOURCE-LOC-NEXT:                      "end": {
// CHECK-SOURCE-LOC-NEXT:                        "line": 22,
// CHECK-SOURCE-LOC-NEXT:                        "column": 17
// CHECK-SOURCE-LOC-NEXT:                      }
// CHECK-SOURCE-LOC-NEXT:                    },
// CHECK-SOURCE-LOC-NEXT:                    "range": [
// CHECK-SOURCE-LOC-NEXT:                      780,
// CHECK-SOURCE-LOC-NEXT:                      786
// CHECK-SOURCE-LOC-NEXT:                    ]
// CHECK-SOURCE-LOC-NEXT:                  }
// CHECK-SOURCE-LOC-NEXT:                ],
// CHECK-SOURCE-LOC-NEXT:                "loc": {
// CHECK-SOURCE-LOC-NEXT:                  "start": {
// CHECK-SOURCE-LOC-NEXT:                    "line": 22,
// CHECK-SOURCE-LOC-NEXT:                    "column": 5
// CHECK-SOURCE-LOC-NEXT:                  },
// CHECK-SOURCE-LOC-NEXT:                  "end": {
// CHECK-SOURCE-LOC-NEXT:                    "line": 22,
// CHECK-SOURCE-LOC-NEXT:                    "column": 18
// CHECK-SOURCE-LOC-NEXT:                  }
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "range": [
// CHECK-SOURCE-LOC-NEXT:                  774,
// CHECK-SOURCE-LOC-NEXT:                  787
// CHECK-SOURCE-LOC-NEXT:                ]
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "directive": null,
// CHECK-SOURCE-LOC-NEXT:              "loc": {
// CHECK-SOURCE-LOC-NEXT:                "start": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 22,
// CHECK-SOURCE-LOC-NEXT:                  "column": 5
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "end": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 22,
// CHECK-SOURCE-LOC-NEXT:                  "column": 19
// CHECK-SOURCE-LOC-NEXT:                }
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "range": [
// CHECK-SOURCE-LOC-NEXT:                774,
// CHECK-SOURCE-LOC-NEXT:                788
// CHECK-SOURCE-LOC-NEXT:              ]
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            {
// CHECK-SOURCE-LOC-NEXT:              "type": "BreakStatement",
// CHECK-SOURCE-LOC-NEXT:              "label": null,
// CHECK-SOURCE-LOC-NEXT:              "loc": {
// CHECK-SOURCE-LOC-NEXT:                "start": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 23,
// CHECK-SOURCE-LOC-NEXT:                  "column": 5
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "end": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 23,
// CHECK-SOURCE-LOC-NEXT:                  "column": 11
// CHECK-SOURCE-LOC-NEXT:                }
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "range": [
// CHECK-SOURCE-LOC-NEXT:                793,
// CHECK-SOURCE-LOC-NEXT:                799
// CHECK-SOURCE-LOC-NEXT:              ]
// CHECK-SOURCE-LOC-NEXT:            }
// CHECK-SOURCE-LOC-NEXT:          ],
// CHECK-SOURCE-LOC-NEXT:          "loc": {
// CHECK-SOURCE-LOC-NEXT:            "start": {
// CHECK-SOURCE-LOC-NEXT:              "line": 21,
// CHECK-SOURCE-LOC-NEXT:              "column": 3
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            "end": {
// CHECK-SOURCE-LOC-NEXT:              "line": 23,
// CHECK-SOURCE-LOC-NEXT:              "column": 11
// CHECK-SOURCE-LOC-NEXT:            }
// CHECK-SOURCE-LOC-NEXT:          },
// CHECK-SOURCE-LOC-NEXT:          "range": [
// CHECK-SOURCE-LOC-NEXT:            762,
// CHECK-SOURCE-LOC-NEXT:            799
// CHECK-SOURCE-LOC-NEXT:          ]
// CHECK-SOURCE-LOC-NEXT:        },
// CHECK-SOURCE-LOC-NEXT:        {
// CHECK-SOURCE-LOC-NEXT:          "type": "SwitchCase",
// CHECK-SOURCE-LOC-NEXT:          "test": null,
// CHECK-SOURCE-LOC-NEXT:          "consequent": [
// CHECK-SOURCE-LOC-NEXT:            {
// CHECK-SOURCE-LOC-NEXT:              "type": "ExpressionStatement",
// CHECK-SOURCE-LOC-NEXT:              "expression": {
// CHECK-SOURCE-LOC-NEXT:                "type": "CallExpression",
// CHECK-SOURCE-LOC-NEXT:                "callee": {
// CHECK-SOURCE-LOC-NEXT:                  "type": "Identifier",
// CHECK-SOURCE-LOC-NEXT:                  "name": "print",
// CHECK-SOURCE-LOC-NEXT:                  "loc": {
// CHECK-SOURCE-LOC-NEXT:                    "start": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 25,
// CHECK-SOURCE-LOC-NEXT:                      "column": 5
// CHECK-SOURCE-LOC-NEXT:                    },
// CHECK-SOURCE-LOC-NEXT:                    "end": {
// CHECK-SOURCE-LOC-NEXT:                      "line": 25,
// CHECK-SOURCE-LOC-NEXT:                      "column": 10
// CHECK-SOURCE-LOC-NEXT:                    }
// CHECK-SOURCE-LOC-NEXT:                  },
// CHECK-SOURCE-LOC-NEXT:                  "range": [
// CHECK-SOURCE-LOC-NEXT:                    815,
// CHECK-SOURCE-LOC-NEXT:                    820
// CHECK-SOURCE-LOC-NEXT:                  ]
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "arguments": [
// CHECK-SOURCE-LOC-NEXT:                  {
// CHECK-SOURCE-LOC-NEXT:                    "type": "CallExpression",
// CHECK-SOURCE-LOC-NEXT:                    "callee": {
// CHECK-SOURCE-LOC-NEXT:                      "type": "Identifier",
// CHECK-SOURCE-LOC-NEXT:                      "name": "foo",
// CHECK-SOURCE-LOC-NEXT:                      "loc": {
// CHECK-SOURCE-LOC-NEXT:                        "start": {
// CHECK-SOURCE-LOC-NEXT:                          "line": 25,
// CHECK-SOURCE-LOC-NEXT:                          "column": 11
// CHECK-SOURCE-LOC-NEXT:                        },
// CHECK-SOURCE-LOC-NEXT:                        "end": {
// CHECK-SOURCE-LOC-NEXT:                          "line": 25,
// CHECK-SOURCE-LOC-NEXT:                          "column": 14
// CHECK-SOURCE-LOC-NEXT:                        }
// CHECK-SOURCE-LOC-NEXT:                      },
// CHECK-SOURCE-LOC-NEXT:                      "range": [
// CHECK-SOURCE-LOC-NEXT:                        821,
// CHECK-SOURCE-LOC-NEXT:                        824
// CHECK-SOURCE-LOC-NEXT:                      ]
// CHECK-SOURCE-LOC-NEXT:                    },
// CHECK-SOURCE-LOC-NEXT:                    "arguments": [],
// CHECK-SOURCE-LOC-NEXT:                    "loc": {
// CHECK-SOURCE-LOC-NEXT:                      "start": {
// CHECK-SOURCE-LOC-NEXT:                        "line": 25,
// CHECK-SOURCE-LOC-NEXT:                        "column": 11
// CHECK-SOURCE-LOC-NEXT:                      },
// CHECK-SOURCE-LOC-NEXT:                      "end": {
// CHECK-SOURCE-LOC-NEXT:                        "line": 25,
// CHECK-SOURCE-LOC-NEXT:                        "column": 16
// CHECK-SOURCE-LOC-NEXT:                      }
// CHECK-SOURCE-LOC-NEXT:                    },
// CHECK-SOURCE-LOC-NEXT:                    "range": [
// CHECK-SOURCE-LOC-NEXT:                      821,
// CHECK-SOURCE-LOC-NEXT:                      826
// CHECK-SOURCE-LOC-NEXT:                    ]
// CHECK-SOURCE-LOC-NEXT:                  }
// CHECK-SOURCE-LOC-NEXT:                ],
// CHECK-SOURCE-LOC-NEXT:                "loc": {
// CHECK-SOURCE-LOC-NEXT:                  "start": {
// CHECK-SOURCE-LOC-NEXT:                    "line": 25,
// CHECK-SOURCE-LOC-NEXT:                    "column": 5
// CHECK-SOURCE-LOC-NEXT:                  },
// CHECK-SOURCE-LOC-NEXT:                  "end": {
// CHECK-SOURCE-LOC-NEXT:                    "line": 25,
// CHECK-SOURCE-LOC-NEXT:                    "column": 17
// CHECK-SOURCE-LOC-NEXT:                  }
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "range": [
// CHECK-SOURCE-LOC-NEXT:                  815,
// CHECK-SOURCE-LOC-NEXT:                  827
// CHECK-SOURCE-LOC-NEXT:                ]
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "directive": null,
// CHECK-SOURCE-LOC-NEXT:              "loc": {
// CHECK-SOURCE-LOC-NEXT:                "start": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 25,
// CHECK-SOURCE-LOC-NEXT:                  "column": 5
// CHECK-SOURCE-LOC-NEXT:                },
// CHECK-SOURCE-LOC-NEXT:                "end": {
// CHECK-SOURCE-LOC-NEXT:                  "line": 25,
// CHECK-SOURCE-LOC-NEXT:                  "column": 18
// CHECK-SOURCE-LOC-NEXT:                }
// CHECK-SOURCE-LOC-NEXT:              },
// CHECK-SOURCE-LOC-NEXT:              "range": [
// CHECK-SOURCE-LOC-NEXT:                815,
// CHECK-SOURCE-LOC-NEXT:                828
// CHECK-SOURCE-LOC-NEXT:              ]
// CHECK-SOURCE-LOC-NEXT:            }
// CHECK-SOURCE-LOC-NEXT:          ],
// CHECK-SOURCE-LOC-NEXT:          "loc": {
// CHECK-SOURCE-LOC-NEXT:            "start": {
// CHECK-SOURCE-LOC-NEXT:              "line": 24,
// CHECK-SOURCE-LOC-NEXT:              "column": 3
// CHECK-SOURCE-LOC-NEXT:            },
// CHECK-SOURCE-LOC-NEXT:            "end": {
// CHECK-SOURCE-LOC-NEXT:              "line": 25,
// CHECK-SOURCE-LOC-NEXT:              "column": 18
// CHECK-SOURCE-LOC-NEXT:            }
// CHECK-SOURCE-LOC-NEXT:          },
// CHECK-SOURCE-LOC-NEXT:          "range": [
// CHECK-SOURCE-LOC-NEXT:            802,
// CHECK-SOURCE-LOC-NEXT:            828
// CHECK-SOURCE-LOC-NEXT:          ]
// CHECK-SOURCE-LOC-NEXT:        }
// CHECK-SOURCE-LOC-NEXT:      ],
// CHECK-SOURCE-LOC-NEXT:      "loc": {
// CHECK-SOURCE-LOC-NEXT:        "start": {
// CHECK-SOURCE-LOC-NEXT:          "line": 17,
// CHECK-SOURCE-LOC-NEXT:          "column": 1
// CHECK-SOURCE-LOC-NEXT:        },
// CHECK-SOURCE-LOC-NEXT:        "end": {
// CHECK-SOURCE-LOC-NEXT:          "line": 26,
// CHECK-SOURCE-LOC-NEXT:          "column": 2
// CHECK-SOURCE-LOC-NEXT:        }
// CHECK-SOURCE-LOC-NEXT:      },
// CHECK-SOURCE-LOC-NEXT:      "range": [
// CHECK-SOURCE-LOC-NEXT:        703,
// CHECK-SOURCE-LOC-NEXT:        830
// CHECK-SOURCE-LOC-NEXT:      ]
// CHECK-SOURCE-LOC-NEXT:    }
// CHECK-SOURCE-LOC-NEXT:  ],
// CHECK-SOURCE-LOC-NEXT:  "loc": {
// CHECK-SOURCE-LOC-NEXT:    "start": {
// CHECK-SOURCE-LOC-NEXT:      "line": 13,
// CHECK-SOURCE-LOC-NEXT:      "column": 1
// CHECK-SOURCE-LOC-NEXT:    },
// CHECK-SOURCE-LOC-NEXT:    "end": {
// CHECK-SOURCE-LOC-NEXT:      "line": 26,
// CHECK-SOURCE-LOC-NEXT:      "column": 2
// CHECK-SOURCE-LOC-NEXT:    }
// CHECK-SOURCE-LOC-NEXT:  },
// CHECK-SOURCE-LOC-NEXT:  "range": [
// CHECK-SOURCE-LOC-NEXT:    659,
// CHECK-SOURCE-LOC-NEXT:    830
// CHECK-SOURCE-LOC-NEXT:  ]
// CHECK-SOURCE-LOC-NEXT:}

// CHECK-FULL:{
// CHECK-FULL-NEXT:  "type": "Program",
// CHECK-FULL-NEXT:  "body": [
// CHECK-FULL-NEXT:    {
// CHECK-FULL-NEXT:      "type": "FunctionDeclaration",
// CHECK-FULL-NEXT:      "id": {
// CHECK-FULL-NEXT:        "type": "Identifier",
// CHECK-FULL-NEXT:        "name": "foo",
// CHECK-FULL-NEXT:        "typeAnnotation": null,
// CHECK-FULL-NEXT:        "optional": false
// CHECK-FULL-NEXT:      },
// CHECK-FULL-NEXT:      "params": [],
// CHECK-FULL-NEXT:      "body": {
// CHECK-FULL-NEXT:        "type": "BlockStatement",
// CHECK-FULL-NEXT:        "body": [
// CHECK-FULL-NEXT:          {
// CHECK-FULL-NEXT:            "type": "ReturnStatement",
// CHECK-FULL-NEXT:            "argument": {
// CHECK-FULL-NEXT:              "type": "CallExpression",
// CHECK-FULL-NEXT:              "callee": {
// CHECK-FULL-NEXT:                "type": "MemberExpression",
// CHECK-FULL-NEXT:                "object": {
// CHECK-FULL-NEXT:                  "type": "Identifier",
// CHECK-FULL-NEXT:                  "name": "Math",
// CHECK-FULL-NEXT:                  "typeAnnotation": null,
// CHECK-FULL-NEXT:                  "optional": false
// CHECK-FULL-NEXT:                },
// CHECK-FULL-NEXT:                "property": {
// CHECK-FULL-NEXT:                  "type": "Identifier",
// CHECK-FULL-NEXT:                  "name": "random",
// CHECK-FULL-NEXT:                  "typeAnnotation": null,
// CHECK-FULL-NEXT:                  "optional": false
// CHECK-FULL-NEXT:                },
// CHECK-FULL-NEXT:                "computed": false
// CHECK-FULL-NEXT:              },
// CHECK-FULL-NEXT:              "typeArguments": null,
// CHECK-FULL-NEXT:              "arguments": []
// CHECK-FULL-NEXT:            }
// CHECK-FULL-NEXT:          }
// CHECK-FULL-NEXT:        ]
// CHECK-FULL-NEXT:      },
// CHECK-FULL-NEXT:      "typeParameters": null,
// CHECK-FULL-NEXT:      "returnType": null,
// CHECK-FULL-NEXT:      "predicate": null,
// CHECK-FULL-NEXT:      "generator": false,
// CHECK-FULL-NEXT:      "async": false
// CHECK-FULL-NEXT:    },
// CHECK-FULL-NEXT:    {
// CHECK-FULL-NEXT:      "type": "SwitchStatement",
// CHECK-FULL-NEXT:      "discriminant": {
// CHECK-FULL-NEXT:        "type": "CallExpression",
// CHECK-FULL-NEXT:        "callee": {
// CHECK-FULL-NEXT:          "type": "Identifier",
// CHECK-FULL-NEXT:          "name": "foo",
// CHECK-FULL-NEXT:          "typeAnnotation": null,
// CHECK-FULL-NEXT:          "optional": false
// CHECK-FULL-NEXT:        },
// CHECK-FULL-NEXT:        "typeArguments": null,
// CHECK-FULL-NEXT:        "arguments": []
// CHECK-FULL-NEXT:      },
// CHECK-FULL-NEXT:      "cases": [
// CHECK-FULL-NEXT:        {
// CHECK-FULL-NEXT:          "type": "SwitchCase",
// CHECK-FULL-NEXT:          "test": {
// CHECK-FULL-NEXT:            "type": "NumericLiteral",
// CHECK-FULL-NEXT:            "value": 3,
// CHECK-FULL-NEXT:            "raw": "3"
// CHECK-FULL-NEXT:          },
// CHECK-FULL-NEXT:          "consequent": [
// CHECK-FULL-NEXT:            {
// CHECK-FULL-NEXT:              "type": "ExpressionStatement",
// CHECK-FULL-NEXT:              "expression": {
// CHECK-FULL-NEXT:                "type": "CallExpression",
// CHECK-FULL-NEXT:                "callee": {
// CHECK-FULL-NEXT:                  "type": "Identifier",
// CHECK-FULL-NEXT:                  "name": "print",
// CHECK-FULL-NEXT:                  "typeAnnotation": null,
// CHECK-FULL-NEXT:                  "optional": false
// CHECK-FULL-NEXT:                },
// CHECK-FULL-NEXT:                "typeArguments": null,
// CHECK-FULL-NEXT:                "arguments": [
// CHECK-FULL-NEXT:                  {
// CHECK-FULL-NEXT:                    "type": "StringLiteral",
// CHECK-FULL-NEXT:                    "value": "fizz"
// CHECK-FULL-NEXT:                  }
// CHECK-FULL-NEXT:                ]
// CHECK-FULL-NEXT:              },
// CHECK-FULL-NEXT:              "directive": null
// CHECK-FULL-NEXT:            },
// CHECK-FULL-NEXT:            {
// CHECK-FULL-NEXT:              "type": "BreakStatement",
// CHECK-FULL-NEXT:              "label": null
// CHECK-FULL-NEXT:            }
// CHECK-FULL-NEXT:          ]
// CHECK-FULL-NEXT:        },
// CHECK-FULL-NEXT:        {
// CHECK-FULL-NEXT:          "type": "SwitchCase",
// CHECK-FULL-NEXT:          "test": {
// CHECK-FULL-NEXT:            "type": "NumericLiteral",
// CHECK-FULL-NEXT:            "value": 5,
// CHECK-FULL-NEXT:            "raw": "5"
// CHECK-FULL-NEXT:          },
// CHECK-FULL-NEXT:          "consequent": [
// CHECK-FULL-NEXT:            {
// CHECK-FULL-NEXT:              "type": "ExpressionStatement",
// CHECK-FULL-NEXT:              "expression": {
// CHECK-FULL-NEXT:                "type": "CallExpression",
// CHECK-FULL-NEXT:                "callee": {
// CHECK-FULL-NEXT:                  "type": "Identifier",
// CHECK-FULL-NEXT:                  "name": "print",
// CHECK-FULL-NEXT:                  "typeAnnotation": null,
// CHECK-FULL-NEXT:                  "optional": false
// CHECK-FULL-NEXT:                },
// CHECK-FULL-NEXT:                "typeArguments": null,
// CHECK-FULL-NEXT:                "arguments": [
// CHECK-FULL-NEXT:                  {
// CHECK-FULL-NEXT:                    "type": "StringLiteral",
// CHECK-FULL-NEXT:                    "value": "buzz"
// CHECK-FULL-NEXT:                  }
// CHECK-FULL-NEXT:                ]
// CHECK-FULL-NEXT:              },
// CHECK-FULL-NEXT:              "directive": null
// CHECK-FULL-NEXT:            },
// CHECK-FULL-NEXT:            {
// CHECK-FULL-NEXT:              "type": "BreakStatement",
// CHECK-FULL-NEXT:              "label": null
// CHECK-FULL-NEXT:            }
// CHECK-FULL-NEXT:          ]
// CHECK-FULL-NEXT:        },
// CHECK-FULL-NEXT:        {
// CHECK-FULL-NEXT:          "type": "SwitchCase",
// CHECK-FULL-NEXT:          "test": null,
// CHECK-FULL-NEXT:          "consequent": [
// CHECK-FULL-NEXT:            {
// CHECK-FULL-NEXT:              "type": "ExpressionStatement",
// CHECK-FULL-NEXT:              "expression": {
// CHECK-FULL-NEXT:                "type": "CallExpression",
// CHECK-FULL-NEXT:                "callee": {
// CHECK-FULL-NEXT:                  "type": "Identifier",
// CHECK-FULL-NEXT:                  "name": "print",
// CHECK-FULL-NEXT:                  "typeAnnotation": null,
// CHECK-FULL-NEXT:                  "optional": false
// CHECK-FULL-NEXT:                },
// CHECK-FULL-NEXT:                "typeArguments": null,
// CHECK-FULL-NEXT:                "arguments": [
// CHECK-FULL-NEXT:                  {
// CHECK-FULL-NEXT:                    "type": "CallExpression",
// CHECK-FULL-NEXT:                    "callee": {
// CHECK-FULL-NEXT:                      "type": "Identifier",
// CHECK-FULL-NEXT:                      "name": "foo",
// CHECK-FULL-NEXT:                      "typeAnnotation": null,
// CHECK-FULL-NEXT:                      "optional": false
// CHECK-FULL-NEXT:                    },
// CHECK-FULL-NEXT:                    "typeArguments": null,
// CHECK-FULL-NEXT:                    "arguments": []
// CHECK-FULL-NEXT:                  }
// CHECK-FULL-NEXT:                ]
// CHECK-FULL-NEXT:              },
// CHECK-FULL-NEXT:              "directive": null
// CHECK-FULL-NEXT:            }
// CHECK-FULL-NEXT:          ]
// CHECK-FULL-NEXT:        }
// CHECK-FULL-NEXT:      ]
// CHECK-FULL-NEXT:    }
// CHECK-FULL-NEXT:  ]
// CHECK-FULL-NEXT:}
