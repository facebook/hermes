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

  [10]: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchArrayPattern",
// CHECK-NEXT:                  "elements": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchLiteralPattern",
// CHECK-NEXT:                      "literal": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 10,
// CHECK-NEXT:                        "raw": "10"
// CHECK-NEXT:                      }
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

  [const y, 1]: y,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchArrayPattern",
// CHECK-NEXT:                  "elements": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchBindingPattern",
// CHECK-NEXT:                      "id": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "y"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "kind": "const"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchLiteralPattern",
// CHECK-NEXT:                      "literal": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 1,
// CHECK-NEXT:                        "raw": "1"
// CHECK-NEXT:                      }
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

  [1, ...]: 1,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchArrayPattern",
// CHECK-NEXT:                  "elements": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchLiteralPattern",
// CHECK-NEXT:                      "literal": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 1,
// CHECK-NEXT:                        "raw": "1"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": {
// CHECK-NEXT:                    "type": "MatchRestPattern",
// CHECK-NEXT:                    "argument": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 1,
// CHECK-NEXT:                  "raw": "1"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  [1, 2, ...const rest]: rest,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchArrayPattern",
// CHECK-NEXT:                  "elements": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchLiteralPattern",
// CHECK-NEXT:                      "literal": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 1,
// CHECK-NEXT:                        "raw": "1"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchLiteralPattern",
// CHECK-NEXT:                      "literal": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 2,
// CHECK-NEXT:                        "raw": "2"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": {
// CHECK-NEXT:                    "type": "MatchRestPattern",
// CHECK-NEXT:                    "argument": {
// CHECK-NEXT:                      "type": "MatchBindingPattern",
// CHECK-NEXT:                      "id": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "rest"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "kind": "const"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "rest"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  [...let rest]: rest,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchArrayPattern",
// CHECK-NEXT:                  "elements": [],
// CHECK-NEXT:                  "rest": {
// CHECK-NEXT:                    "type": "MatchRestPattern",
// CHECK-NEXT:                    "argument": {
// CHECK-NEXT:                      "type": "MatchBindingPattern",
// CHECK-NEXT:                      "id": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "rest"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "kind": "let"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "rest"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  [...var rest]: rest,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchArrayPattern",
// CHECK-NEXT:                  "elements": [],
// CHECK-NEXT:                  "rest": {
// CHECK-NEXT:                    "type": "MatchRestPattern",
// CHECK-NEXT:                    "argument": {
// CHECK-NEXT:                      "type": "MatchBindingPattern",
// CHECK-NEXT:                      "id": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "rest"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "kind": "var"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "rest"
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  [{nested: [1, const x]}]: x,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchArrayPattern",
// CHECK-NEXT:                  "elements": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "MatchObjectPattern",
// CHECK-NEXT:                      "properties": [
// CHECK-NEXT:                        {
// CHECK-NEXT:                          "type": "MatchObjectPatternProperty",
// CHECK-NEXT:                          "key": {
// CHECK-NEXT:                            "type": "Identifier",
// CHECK-NEXT:                            "name": "nested"
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "pattern": {
// CHECK-NEXT:                            "type": "MatchArrayPattern",
// CHECK-NEXT:                            "elements": [
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "MatchLiteralPattern",
// CHECK-NEXT:                                "literal": {
// CHECK-NEXT:                                  "type": "NumericLiteral",
// CHECK-NEXT:                                  "value": 1,
// CHECK-NEXT:                                  "raw": "1"
// CHECK-NEXT:                                }
// CHECK-NEXT:                              },
// CHECK-NEXT:                              {
// CHECK-NEXT:                                "type": "MatchBindingPattern",
// CHECK-NEXT:                                "id": {
// CHECK-NEXT:                                  "type": "Identifier",
// CHECK-NEXT:                                  "name": "x"
// CHECK-NEXT:                                },
// CHECK-NEXT:                                "kind": "const"
// CHECK-NEXT:                              }
// CHECK-NEXT:                            ],
// CHECK-NEXT:                            "rest": null
// CHECK-NEXT:                          },
// CHECK-NEXT:                          "shorthand": false
// CHECK-NEXT:                        }
// CHECK-NEXT:                      ],
// CHECK-NEXT:                      "rest": null
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ],
// CHECK-NEXT:                  "rest": null
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
