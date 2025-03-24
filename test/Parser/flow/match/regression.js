/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * Here we test that none of the following parse as match expressions or statements.
 * Each case should parse as regular identifier with the name `match`.
 */

// RUN: %hermesc -parse-flow -Xparse-flow-match -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines
// RUN: %hermesc -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

type match = number;

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "TypeAlias",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "match"
// CHECK-NEXT:      },
// CHECK-NEXT:      "typeParameters": null,
// CHECK-NEXT:      "right": {
// CHECK-NEXT:        "type": "NumberTypeAnnotation"
// CHECK-NEXT:      }
// CHECK-NEXT:    },

{
  const match = 1;
}

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "BlockStatement",
// CHECK-NEXT:      "body": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclaration",
// CHECK-NEXT:          "kind": "const",
// CHECK-NEXT:          "declarations": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "VariableDeclarator",
// CHECK-NEXT:              "init": {
// CHECK-NEXT:                "type": "NumericLiteral",
// CHECK-NEXT:                "value": 1,
// CHECK-NEXT:                "raw": "1"
// CHECK-NEXT:              },
// CHECK-NEXT:              "id": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "match"
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

function match(match: match) {}

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "FunctionDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "match"
// CHECK-NEXT:      },
// CHECK-NEXT:      "params": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "match",
// CHECK-NEXT:          "typeAnnotation": {
// CHECK-NEXT:            "type": "TypeAnnotation",
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "GenericTypeAnnotation",
// CHECK-NEXT:              "id": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "match"
// CHECK-NEXT:              },
// CHECK-NEXT:              "typeParameters": null
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ],
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "BlockStatement",
// CHECK-NEXT:        "body": []
// CHECK-NEXT:      },
// CHECK-NEXT:      "generator": false,
// CHECK-NEXT:      "async": false
// CHECK-NEXT:    },

const a = match(1);

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "CallExpression",
// CHECK-NEXT:            "callee": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "match"
// CHECK-NEXT:            },
// CHECK-NEXT:            "arguments": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "NumericLiteral",
// CHECK-NEXT:                "value": 1,
// CHECK-NEXT:                "raw": "1"
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "a"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

const b = match(1).f();

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "CallExpression",
// CHECK-NEXT:            "callee": {
// CHECK-NEXT:              "type": "MemberExpression",
// CHECK-NEXT:              "object": {
// CHECK-NEXT:                "type": "CallExpression",
// CHECK-NEXT:                "callee": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "match"
// CHECK-NEXT:                },
// CHECK-NEXT:                "arguments": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 1,
// CHECK-NEXT:                    "raw": "1"
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              "property": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "f"
// CHECK-NEXT:              },
// CHECK-NEXT:              "computed": false
// CHECK-NEXT:            },
// CHECK-NEXT:            "arguments": []
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "b"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

const c = match(1)
{
  // block statement
}

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "CallExpression",
// CHECK-NEXT:            "callee": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "match"
// CHECK-NEXT:            },
// CHECK-NEXT:            "arguments": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "NumericLiteral",
// CHECK-NEXT:                "value": 1,
// CHECK-NEXT:                "raw": "1"
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "c"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "BlockStatement",
// CHECK-NEXT:      "body": []
// CHECK-NEXT:    },

const d = match(1) + 2;

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "BinaryExpression",
// CHECK-NEXT:            "left": {
// CHECK-NEXT:              "type": "CallExpression",
// CHECK-NEXT:              "callee": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "match"
// CHECK-NEXT:              },
// CHECK-NEXT:              "arguments": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                }
// CHECK-NEXT:              ]
// CHECK-NEXT:            },
// CHECK-NEXT:            "right": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 2,
// CHECK-NEXT:              "raw": "2"
// CHECK-NEXT:            },
// CHECK-NEXT:            "operator": "+"
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "d"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

const e = match(1)[2];

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "MemberExpression",
// CHECK-NEXT:            "object": {
// CHECK-NEXT:              "type": "CallExpression",
// CHECK-NEXT:              "callee": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "match"
// CHECK-NEXT:              },
// CHECK-NEXT:              "arguments": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                }
// CHECK-NEXT:              ]
// CHECK-NEXT:            },
// CHECK-NEXT:            "property": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 2,
// CHECK-NEXT:              "raw": "2"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": true
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "e"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

const f = match(1) ? 2 : 3;

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "ConditionalExpression",
// CHECK-NEXT:            "test": {
// CHECK-NEXT:              "type": "CallExpression",
// CHECK-NEXT:              "callee": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "match"
// CHECK-NEXT:              },
// CHECK-NEXT:              "arguments": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                }
// CHECK-NEXT:              ]
// CHECK-NEXT:            },
// CHECK-NEXT:            "alternate": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 3,
// CHECK-NEXT:              "raw": "3"
// CHECK-NEXT:            },
// CHECK-NEXT:            "consequent": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 2,
// CHECK-NEXT:              "raw": "2"
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "f"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

const g = match(1) < 2;

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "BinaryExpression",
// CHECK-NEXT:            "left": {
// CHECK-NEXT:              "type": "CallExpression",
// CHECK-NEXT:              "callee": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "match"
// CHECK-NEXT:              },
// CHECK-NEXT:              "arguments": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                }
// CHECK-NEXT:              ]
// CHECK-NEXT:            },
// CHECK-NEXT:            "right": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 2,
// CHECK-NEXT:              "raw": "2"
// CHECK-NEXT:            },
// CHECK-NEXT:            "operator": "<"
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "g"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

const h = match(1) && 2;

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "LogicalExpression",
// CHECK-NEXT:            "left": {
// CHECK-NEXT:              "type": "CallExpression",
// CHECK-NEXT:              "callee": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "match"
// CHECK-NEXT:              },
// CHECK-NEXT:              "arguments": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                }
// CHECK-NEXT:              ]
// CHECK-NEXT:            },
// CHECK-NEXT:            "right": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 2,
// CHECK-NEXT:              "raw": "2"
// CHECK-NEXT:            },
// CHECK-NEXT:            "operator": "&&"
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "h"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

const i = match();

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "CallExpression",
// CHECK-NEXT:            "callee": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "match"
// CHECK-NEXT:            },
// CHECK-NEXT:            "arguments": []
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "i"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

const j = match(...b);

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "VariableDeclaration",
// CHECK-NEXT:      "kind": "const",
// CHECK-NEXT:      "declarations": [
// CHECK-NEXT:        {
// CHECK-NEXT:          "type": "VariableDeclarator",
// CHECK-NEXT:          "init": {
// CHECK-NEXT:            "type": "CallExpression",
// CHECK-NEXT:            "callee": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "match"
// CHECK-NEXT:            },
// CHECK-NEXT:            "arguments": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "SpreadElement",
// CHECK-NEXT:                "argument": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "b"
// CHECK-NEXT:                }
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "id": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "j"
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      ]
// CHECK-NEXT:    },

match(1);

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "CallExpression",
// CHECK-NEXT:        "callee": {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "match"
// CHECK-NEXT:        },
// CHECK-NEXT:        "arguments": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "NumericLiteral",
// CHECK-NEXT:            "value": 1,
// CHECK-NEXT:            "raw": "1"
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },

match(1).f();

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "CallExpression",
// CHECK-NEXT:        "callee": {
// CHECK-NEXT:          "type": "MemberExpression",
// CHECK-NEXT:          "object": {
// CHECK-NEXT:            "type": "CallExpression",
// CHECK-NEXT:            "callee": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "match"
// CHECK-NEXT:            },
// CHECK-NEXT:            "arguments": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "NumericLiteral",
// CHECK-NEXT:                "value": 1,
// CHECK-NEXT:                "raw": "1"
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "property": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "f"
// CHECK-NEXT:          },
// CHECK-NEXT:          "computed": false
// CHECK-NEXT:        },
// CHECK-NEXT:        "arguments": []
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },

match(1)
{
  // block statement
}

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "CallExpression",
// CHECK-NEXT:        "callee": {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "match"
// CHECK-NEXT:        },
// CHECK-NEXT:        "arguments": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "NumericLiteral",
// CHECK-NEXT:            "value": 1,
// CHECK-NEXT:            "raw": "1"
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "BlockStatement",
// CHECK-NEXT:      "body": []
// CHECK-NEXT:    },

match(1) + 2;

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "BinaryExpression",
// CHECK-NEXT:        "left": {
// CHECK-NEXT:          "type": "CallExpression",
// CHECK-NEXT:          "callee": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "match"
// CHECK-NEXT:          },
// CHECK-NEXT:          "arguments": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 1,
// CHECK-NEXT:              "raw": "1"
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        },
// CHECK-NEXT:        "right": {
// CHECK-NEXT:          "type": "NumericLiteral",
// CHECK-NEXT:          "value": 2,
// CHECK-NEXT:          "raw": "2"
// CHECK-NEXT:        },
// CHECK-NEXT:        "operator": "+"
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },

match(1)[2];

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "MemberExpression",
// CHECK-NEXT:        "object": {
// CHECK-NEXT:          "type": "CallExpression",
// CHECK-NEXT:          "callee": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "match"
// CHECK-NEXT:          },
// CHECK-NEXT:          "arguments": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 1,
// CHECK-NEXT:              "raw": "1"
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        },
// CHECK-NEXT:        "property": {
// CHECK-NEXT:          "type": "NumericLiteral",
// CHECK-NEXT:          "value": 2,
// CHECK-NEXT:          "raw": "2"
// CHECK-NEXT:        },
// CHECK-NEXT:        "computed": true
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },

match(1) ? 2 : 3;

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "ConditionalExpression",
// CHECK-NEXT:        "test": {
// CHECK-NEXT:          "type": "CallExpression",
// CHECK-NEXT:          "callee": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "match"
// CHECK-NEXT:          },
// CHECK-NEXT:          "arguments": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 1,
// CHECK-NEXT:              "raw": "1"
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        },
// CHECK-NEXT:        "alternate": {
// CHECK-NEXT:          "type": "NumericLiteral",
// CHECK-NEXT:          "value": 3,
// CHECK-NEXT:          "raw": "3"
// CHECK-NEXT:        },
// CHECK-NEXT:        "consequent": {
// CHECK-NEXT:          "type": "NumericLiteral",
// CHECK-NEXT:          "value": 2,
// CHECK-NEXT:          "raw": "2"
// CHECK-NEXT:        }
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },

match(1) < 2;

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "BinaryExpression",
// CHECK-NEXT:        "left": {
// CHECK-NEXT:          "type": "CallExpression",
// CHECK-NEXT:          "callee": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "match"
// CHECK-NEXT:          },
// CHECK-NEXT:          "arguments": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 1,
// CHECK-NEXT:              "raw": "1"
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        },
// CHECK-NEXT:        "right": {
// CHECK-NEXT:          "type": "NumericLiteral",
// CHECK-NEXT:          "value": 2,
// CHECK-NEXT:          "raw": "2"
// CHECK-NEXT:        },
// CHECK-NEXT:        "operator": "<"
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },

match(1) && 2;

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "LogicalExpression",
// CHECK-NEXT:        "left": {
// CHECK-NEXT:          "type": "CallExpression",
// CHECK-NEXT:          "callee": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "match"
// CHECK-NEXT:          },
// CHECK-NEXT:          "arguments": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 1,
// CHECK-NEXT:              "raw": "1"
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        },
// CHECK-NEXT:        "right": {
// CHECK-NEXT:          "type": "NumericLiteral",
// CHECK-NEXT:          "value": 2,
// CHECK-NEXT:          "raw": "2"
// CHECK-NEXT:        },
// CHECK-NEXT:        "operator": "&&"
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },

match();

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "CallExpression",
// CHECK-NEXT:        "callee": {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "match"
// CHECK-NEXT:        },
// CHECK-NEXT:        "arguments": []
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    },

match(...b);

// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "CallExpression",
// CHECK-NEXT:        "callee": {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "match"
// CHECK-NEXT:        },
// CHECK-NEXT:        "arguments": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "SpreadElement",
// CHECK-NEXT:            "argument": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "b"
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
