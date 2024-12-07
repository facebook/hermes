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

  foo.bar: true,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchMemberPattern",
// CHECK-NEXT:                  "base": {
// CHECK-NEXT:                    "type": "MatchIdentifierPattern",
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "foo"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "property": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "bar"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BooleanLiteral",
// CHECK-NEXT:                  "value": true
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  foo[1]: true,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchMemberPattern",
// CHECK-NEXT:                  "base": {
// CHECK-NEXT:                    "type": "MatchIdentifierPattern",
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "foo"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "property": {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 1,
// CHECK-NEXT:                    "raw": "1"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BooleanLiteral",
// CHECK-NEXT:                  "value": true
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  foo[1n]: true,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchMemberPattern",
// CHECK-NEXT:                  "base": {
// CHECK-NEXT:                    "type": "MatchIdentifierPattern",
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "foo"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "property": {
// CHECK-NEXT:                    "type": "BigIntLiteral",
// CHECK-NEXT:                    "bigint": "1n"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BooleanLiteral",
// CHECK-NEXT:                  "value": true
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  foo["bar"]: true,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchMemberPattern",
// CHECK-NEXT:                  "base": {
// CHECK-NEXT:                    "type": "MatchIdentifierPattern",
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "foo"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "property": {
// CHECK-NEXT:                    "type": "StringLiteral",
// CHECK-NEXT:                    "value": "bar"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BooleanLiteral",
// CHECK-NEXT:                  "value": true
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  foo.bar[1]: true,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchMemberPattern",
// CHECK-NEXT:                  "base": {
// CHECK-NEXT:                    "type": "MatchMemberPattern",
// CHECK-NEXT:                    "base": {
// CHECK-NEXT:                      "type": "MatchIdentifierPattern",
// CHECK-NEXT:                      "id": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "foo"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "property": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "bar"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "property": {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 1,
// CHECK-NEXT:                    "raw": "1"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BooleanLiteral",
// CHECK-NEXT:                  "value": true
// CHECK-NEXT:                },
// CHECK-NEXT:                "guard": null
// CHECK-NEXT:              },

  foo[1].bar["baz"]: true,

// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "MatchExpressionCase",
// CHECK-NEXT:                "pattern": {
// CHECK-NEXT:                  "type": "MatchMemberPattern",
// CHECK-NEXT:                  "base": {
// CHECK-NEXT:                    "type": "MatchMemberPattern",
// CHECK-NEXT:                    "base": {
// CHECK-NEXT:                      "type": "MatchMemberPattern",
// CHECK-NEXT:                      "base": {
// CHECK-NEXT:                        "type": "MatchIdentifierPattern",
// CHECK-NEXT:                        "id": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "foo"
// CHECK-NEXT:                        }
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "property": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 1,
// CHECK-NEXT:                        "raw": "1"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "property": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "bar"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "property": {
// CHECK-NEXT:                    "type": "StringLiteral",
// CHECK-NEXT:                    "value": "baz"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                "body": {
// CHECK-NEXT:                  "type": "BooleanLiteral",
// CHECK-NEXT:                  "value": true
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
// CHECK-NEXT:      ]
// CHECK-NEXT:    }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
