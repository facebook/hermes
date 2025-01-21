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

  {foo: 1, bar: 2}: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchObjectPattern",
// CHECK-NEXT:                  "properties": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "foo"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchLiteralPattern",
// CHECK-NEXT:                        "literal": {
// CHECK-NEXT:                          "type": "NumericLiteral",
// CHECK-NEXT:                          "value": 1,
// CHECK-NEXT:                          "raw": "1"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": false
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "bar"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchLiteralPattern",
// CHECK-NEXT:                        "literal": {
// CHECK-NEXT:                          "type": "NumericLiteral",
// CHECK-NEXT:                          "value": 2,
// CHECK-NEXT:                          "raw": "2"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": false
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": null
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  {'foo': 1}: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchObjectPattern",
// CHECK-NEXT:                  "properties": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "StringLiteral",
// CHECK-NEXT:                        "value": "foo"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchLiteralPattern",
// CHECK-NEXT:                        "literal": {
// CHECK-NEXT:                          "type": "NumericLiteral",
// CHECK-NEXT:                          "value": 1,
// CHECK-NEXT:                          "raw": "1"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": false
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": null
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  {111: true}: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchObjectPattern",
// CHECK-NEXT:                  "properties": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 111,
// CHECK-NEXT:                        "raw": "111"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchLiteralPattern",
// CHECK-NEXT:                        "literal": {
// CHECK-NEXT:                          "type": "BooleanLiteral",
// CHECK-NEXT:                          "value": true
// CHECK-NEXT:                        }
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": false
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": null
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  {foo: const y}: y,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchObjectPattern",
// CHECK-NEXT:                  "properties": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "foo"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchBindingPattern",
// CHECK-NEXT:                        "id": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "y"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "kind": "const"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": false
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": null
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "y"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  {const x, let y, var z}: y,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchObjectPattern",
// CHECK-NEXT:                  "properties": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "x"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchBindingPattern",
// CHECK-NEXT:                        "id": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "x"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "kind": "const"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": true
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "y"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchBindingPattern",
// CHECK-NEXT:                        "id": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "y"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "kind": "let"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": true
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "z"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchBindingPattern",
// CHECK-NEXT:                        "id": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "z"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "kind": "var"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": true
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": null
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "y"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  {const x, ...const y}: y,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchObjectPattern",
// CHECK-NEXT:                  "properties": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "x"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchBindingPattern",
// CHECK-NEXT:                        "id": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "x"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "kind": "const"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": true
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": {
// CHECK-NEXT:                    "type": "MatchRestPattern",
// CHECK-NEXT:                    "argument": {
// CHECK-NEXT:                      "type": "MatchBindingPattern",
// CHECK-NEXT:                      "id": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "y"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "kind": "const"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "y"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  {const x, ...let y}: y,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchObjectPattern",
// CHECK-NEXT:                  "properties": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "x"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchBindingPattern",
// CHECK-NEXT:                        "id": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "x"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "kind": "const"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": true
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": {
// CHECK-NEXT:                    "type": "MatchRestPattern",
// CHECK-NEXT:                    "argument": {
// CHECK-NEXT:                      "type": "MatchBindingPattern",
// CHECK-NEXT:                      "id": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "y"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "kind": "let"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "y"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  {const x, ...}: x,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchObjectPattern",
// CHECK-NEXT:                  "properties": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                      "key": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "x"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "pattern": {
// CHECK-NEXT:                        "type": "MatchBindingPattern",
// CHECK-NEXT:                        "id": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "x"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "kind": "const"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "shorthand": true
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": {
// CHECK-NEXT:                    "type": "MatchRestPattern",
// CHECK-NEXT:                    "argument": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "x"
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

// CHECK-NEXT:   ]
// CHECK-NEXT: }
