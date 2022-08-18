/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -hermes-parser -dump-ast %s | %FileCheck %s --match-full-lines

var r, a, b, c, d, e;
//CHECK: {
//CHECK-NEXT:   "type": "Program",
//CHECK-NEXT:   "body": [
//CHECK-NEXT:     {
//CHECK-NEXT:       "type": "VariableDeclaration",
//CHECK-NEXT:       "kind": "var",
//CHECK-NEXT:       "declarations": [
//CHECK-NEXT:         {
//CHECK-NEXT:           "type": "VariableDeclarator",
//CHECK-NEXT:           "init": null,
//CHECK-NEXT:           "id": {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "r"
//CHECK-NEXT:           }
//CHECK-NEXT:         },
//CHECK-NEXT:         {
//CHECK-NEXT:           "type": "VariableDeclarator",
//CHECK-NEXT:           "init": null,
//CHECK-NEXT:           "id": {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "a"
//CHECK-NEXT:           }
//CHECK-NEXT:         },
//CHECK-NEXT:         {
//CHECK-NEXT:           "type": "VariableDeclarator",
//CHECK-NEXT:           "init": null,
//CHECK-NEXT:           "id": {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "b"
//CHECK-NEXT:           }
//CHECK-NEXT:         },
//CHECK-NEXT:         {
//CHECK-NEXT:           "type": "VariableDeclarator",
//CHECK-NEXT:           "init": null,
//CHECK-NEXT:           "id": {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "c"
//CHECK-NEXT:           }
//CHECK-NEXT:         },
//CHECK-NEXT:         {
//CHECK-NEXT:           "type": "VariableDeclarator",
//CHECK-NEXT:           "init": null,
//CHECK-NEXT:           "id": {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "d"
//CHECK-NEXT:           }
//CHECK-NEXT:         },
//CHECK-NEXT:         {
//CHECK-NEXT:           "type": "VariableDeclarator",
//CHECK-NEXT:           "init": null,
//CHECK-NEXT:           "id": {
//CHECK-NEXT:             "type": "Identifier",
//CHECK-NEXT:             "name": "e"
//CHECK-NEXT:           }
//CHECK-NEXT:         }
//CHECK-NEXT:       ]
//CHECK-NEXT:     },

r = a + b * c - d;
//CHECK-NEXT:    {
//CHECK-NEXT:      "type": "ExpressionStatement",
//CHECK-NEXT:      "expression": {
//CHECK-NEXT:        "type": "AssignmentExpression",
//CHECK-NEXT:        "operator": "=",
//CHECK-NEXT:        "left": {
//CHECK-NEXT:          "type": "Identifier",
//CHECK-NEXT:          "name": "r"
//CHECK-NEXT:        },
//CHECK-NEXT:        "right": {
//CHECK-NEXT:          "type": "BinaryExpression",
//CHECK-NEXT:          "left": {
//CHECK-NEXT:            "type": "BinaryExpression",
//CHECK-NEXT:            "left": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "a"
//CHECK-NEXT:            },
//CHECK-NEXT:            "right": {
//CHECK-NEXT:              "type": "BinaryExpression",
//CHECK-NEXT:              "left": {
//CHECK-NEXT:                "type": "Identifier",
//CHECK-NEXT:                "name": "b"
//CHECK-NEXT:              },
//CHECK-NEXT:              "right": {
//CHECK-NEXT:                "type": "Identifier",
//CHECK-NEXT:                "name": "c"
//CHECK-NEXT:              },
//CHECK-NEXT:              "operator": "*"
//CHECK-NEXT:            },
//CHECK-NEXT:            "operator": "+"
//CHECK-NEXT:          },
//CHECK-NEXT:          "right": {
//CHECK-NEXT:            "type": "Identifier",
//CHECK-NEXT:            "name": "d"
//CHECK-NEXT:          },
//CHECK-NEXT:          "operator": "-"
//CHECK-NEXT:        }
//CHECK-NEXT:      },
//CHECK-NEXT:      "directive": null
//CHECK-NEXT:    },

r = a + b * c / d + e;
//CHECK-NEXT:    {
//CHECK-NEXT:      "type": "ExpressionStatement",
//CHECK-NEXT:      "expression": {
//CHECK-NEXT:        "type": "AssignmentExpression",
//CHECK-NEXT:        "operator": "=",
//CHECK-NEXT:        "left": {
//CHECK-NEXT:          "type": "Identifier",
//CHECK-NEXT:          "name": "r"
//CHECK-NEXT:        },
//CHECK-NEXT:        "right": {
//CHECK-NEXT:          "type": "BinaryExpression",
//CHECK-NEXT:          "left": {
//CHECK-NEXT:            "type": "BinaryExpression",
//CHECK-NEXT:            "left": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "a"
//CHECK-NEXT:            },
//CHECK-NEXT:            "right": {
//CHECK-NEXT:              "type": "BinaryExpression",
//CHECK-NEXT:              "left": {
//CHECK-NEXT:                "type": "BinaryExpression",
//CHECK-NEXT:                "left": {
//CHECK-NEXT:                  "type": "Identifier",
//CHECK-NEXT:                  "name": "b"
//CHECK-NEXT:                },
//CHECK-NEXT:                "right": {
//CHECK-NEXT:                  "type": "Identifier",
//CHECK-NEXT:                  "name": "c"
//CHECK-NEXT:                },
//CHECK-NEXT:                "operator": "*"
//CHECK-NEXT:              },
//CHECK-NEXT:              "right": {
//CHECK-NEXT:                "type": "Identifier",
//CHECK-NEXT:                "name": "d"
//CHECK-NEXT:              },
//CHECK-NEXT:              "operator": "\/"
//CHECK-NEXT:            },
//CHECK-NEXT:            "operator": "+"
//CHECK-NEXT:          },
//CHECK-NEXT:          "right": {
//CHECK-NEXT:            "type": "Identifier",
//CHECK-NEXT:            "name": "e"
//CHECK-NEXT:          },
//CHECK-NEXT:          "operator": "+"
//CHECK-NEXT:        }
//CHECK-NEXT:      },
//CHECK-NEXT:      "directive": null
//CHECK-NEXT:    },

r = a * b + c * d;
//CHECK-NEXT:    {
//CHECK-NEXT:      "type": "ExpressionStatement",
//CHECK-NEXT:      "expression": {
//CHECK-NEXT:        "type": "AssignmentExpression",
//CHECK-NEXT:        "operator": "=",
//CHECK-NEXT:        "left": {
//CHECK-NEXT:          "type": "Identifier",
//CHECK-NEXT:          "name": "r"
//CHECK-NEXT:        },
//CHECK-NEXT:        "right": {
//CHECK-NEXT:          "type": "BinaryExpression",
//CHECK-NEXT:          "left": {
//CHECK-NEXT:            "type": "BinaryExpression",
//CHECK-NEXT:            "left": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "a"
//CHECK-NEXT:            },
//CHECK-NEXT:            "right": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "b"
//CHECK-NEXT:            },
//CHECK-NEXT:            "operator": "*"
//CHECK-NEXT:          },
//CHECK-NEXT:          "right": {
//CHECK-NEXT:            "type": "BinaryExpression",
//CHECK-NEXT:            "left": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "c"
//CHECK-NEXT:            },
//CHECK-NEXT:            "right": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "d"
//CHECK-NEXT:            },
//CHECK-NEXT:            "operator": "*"
//CHECK-NEXT:          },
//CHECK-NEXT:          "operator": "+"
//CHECK-NEXT:        }
//CHECK-NEXT:      },
//CHECK-NEXT:      "directive": null
//CHECK-NEXT:    },

r = a * b + c - d;
//CHECK-NEXT:    {
//CHECK-NEXT:      "type": "ExpressionStatement",
//CHECK-NEXT:      "expression": {
//CHECK-NEXT:        "type": "AssignmentExpression",
//CHECK-NEXT:        "operator": "=",
//CHECK-NEXT:        "left": {
//CHECK-NEXT:          "type": "Identifier",
//CHECK-NEXT:          "name": "r"
//CHECK-NEXT:        },
//CHECK-NEXT:        "right": {
//CHECK-NEXT:          "type": "BinaryExpression",
//CHECK-NEXT:          "left": {
//CHECK-NEXT:            "type": "BinaryExpression",
//CHECK-NEXT:            "left": {
//CHECK-NEXT:              "type": "BinaryExpression",
//CHECK-NEXT:              "left": {
//CHECK-NEXT:                "type": "Identifier",
//CHECK-NEXT:                "name": "a"
//CHECK-NEXT:              },
//CHECK-NEXT:              "right": {
//CHECK-NEXT:                "type": "Identifier",
//CHECK-NEXT:                "name": "b"
//CHECK-NEXT:              },
//CHECK-NEXT:              "operator": "*"
//CHECK-NEXT:            },
//CHECK-NEXT:            "right": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "c"
//CHECK-NEXT:            },
//CHECK-NEXT:            "operator": "+"
//CHECK-NEXT:          },
//CHECK-NEXT:          "right": {
//CHECK-NEXT:            "type": "Identifier",
//CHECK-NEXT:            "name": "d"
//CHECK-NEXT:          },
//CHECK-NEXT:          "operator": "-"
//CHECK-NEXT:        }
//CHECK-NEXT:      },
//CHECK-NEXT:      "directive": null
//CHECK-NEXT:    },

// 'in' precedence is handled separately from TokenKinds.def and easy to miss.
r = a in b !== c in d;
//CHECK-NEXT:    {
//CHECK-NEXT:      "type": "ExpressionStatement",
//CHECK-NEXT:      "expression": {
//CHECK-NEXT:        "type": "AssignmentExpression",
//CHECK-NEXT:        "operator": "=",
//CHECK-NEXT:        "left": {
//CHECK-NEXT:          "type": "Identifier",
//CHECK-NEXT:          "name": "r"
//CHECK-NEXT:        },
//CHECK-NEXT:        "right": {
//CHECK-NEXT:          "type": "BinaryExpression",
//CHECK-NEXT:          "left": {
//CHECK-NEXT:            "type": "BinaryExpression",
//CHECK-NEXT:            "left": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "a"
//CHECK-NEXT:            },
//CHECK-NEXT:            "right": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "b"
//CHECK-NEXT:            },
//CHECK-NEXT:            "operator": "in"
//CHECK-NEXT:          },
//CHECK-NEXT:          "right": {
//CHECK-NEXT:            "type": "BinaryExpression",
//CHECK-NEXT:            "left": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "c"
//CHECK-NEXT:            },
//CHECK-NEXT:            "right": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "d"
//CHECK-NEXT:            },
//CHECK-NEXT:            "operator": "in"
//CHECK-NEXT:          },
//CHECK-NEXT:          "operator": "!=="
//CHECK-NEXT:        }
//CHECK-NEXT:      },
//CHECK-NEXT:      "directive": null
//CHECK-NEXT:    }

//CHECK-NEXT:  ]
//CHECK-NEXT:}
