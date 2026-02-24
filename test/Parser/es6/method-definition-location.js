/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -dump-source-location=loc --pretty-json %s | %FileCheckOrRegen %s --match-full-lines

// Make sure that the FunctionExpressions start at the '('.
({
  foo/*           */() {},
  get foo/*       */() {},
  set foo/*       */(v) {},
});

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "ObjectExpression",
// CHECK-NEXT:        "properties": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "Property",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "foo",
// CHECK-NEXT:              "loc": {
// CHECK-NEXT:                "start": {
// CHECK-NEXT:                  "line": 12,
// CHECK-NEXT:                  "column": 3
// CHECK-NEXT:                },
// CHECK-NEXT:                "end": {
// CHECK-NEXT:                  "line": 12,
// CHECK-NEXT:                  "column": 6
// CHECK-NEXT:                }
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": [],
// CHECK-NEXT:                "loc": {
// CHECK-NEXT:                  "start": {
// CHECK-NEXT:                    "line": 12,
// CHECK-NEXT:                    "column": 24
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "end": {
// CHECK-NEXT:                    "line": 12,
// CHECK-NEXT:                    "column": 26
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false,
// CHECK-NEXT:              "loc": {
// CHECK-NEXT:                "start": {
// CHECK-NEXT:                  "line": 12,
// CHECK-NEXT:                  "column": 21
// CHECK-NEXT:                },
// CHECK-NEXT:                "end": {
// CHECK-NEXT:                  "line": 12,
// CHECK-NEXT:                  "column": 26
// CHECK-NEXT:                }
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "init",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "method": true,
// CHECK-NEXT:            "shorthand": false,
// CHECK-NEXT:            "loc": {
// CHECK-NEXT:              "start": {
// CHECK-NEXT:                "line": 12,
// CHECK-NEXT:                "column": 3
// CHECK-NEXT:              },
// CHECK-NEXT:              "end": {
// CHECK-NEXT:                "line": 12,
// CHECK-NEXT:                "column": 26
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "Property",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "foo",
// CHECK-NEXT:              "loc": {
// CHECK-NEXT:                "start": {
// CHECK-NEXT:                  "line": 13,
// CHECK-NEXT:                  "column": 7
// CHECK-NEXT:                },
// CHECK-NEXT:                "end": {
// CHECK-NEXT:                  "line": 13,
// CHECK-NEXT:                  "column": 10
// CHECK-NEXT:                }
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": [],
// CHECK-NEXT:                "loc": {
// CHECK-NEXT:                  "start": {
// CHECK-NEXT:                    "line": 13,
// CHECK-NEXT:                    "column": 24
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "end": {
// CHECK-NEXT:                    "line": 13,
// CHECK-NEXT:                    "column": 26
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false,
// CHECK-NEXT:              "loc": {
// CHECK-NEXT:                "start": {
// CHECK-NEXT:                  "line": 13,
// CHECK-NEXT:                  "column": 21
// CHECK-NEXT:                },
// CHECK-NEXT:                "end": {
// CHECK-NEXT:                  "line": 13,
// CHECK-NEXT:                  "column": 26
// CHECK-NEXT:                }
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "get",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "method": false,
// CHECK-NEXT:            "shorthand": false,
// CHECK-NEXT:            "loc": {
// CHECK-NEXT:              "start": {
// CHECK-NEXT:                "line": 13,
// CHECK-NEXT:                "column": 3
// CHECK-NEXT:              },
// CHECK-NEXT:              "end": {
// CHECK-NEXT:                "line": 13,
// CHECK-NEXT:                "column": 26
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "Property",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "foo",
// CHECK-NEXT:              "loc": {
// CHECK-NEXT:                "start": {
// CHECK-NEXT:                  "line": 14,
// CHECK-NEXT:                  "column": 7
// CHECK-NEXT:                },
// CHECK-NEXT:                "end": {
// CHECK-NEXT:                  "line": 14,
// CHECK-NEXT:                  "column": 10
// CHECK-NEXT:                }
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "v",
// CHECK-NEXT:                  "loc": {
// CHECK-NEXT:                    "start": {
// CHECK-NEXT:                      "line": 14,
// CHECK-NEXT:                      "column": 22
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "end": {
// CHECK-NEXT:                      "line": 14,
// CHECK-NEXT:                      "column": 23
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              ],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": [],
// CHECK-NEXT:                "loc": {
// CHECK-NEXT:                  "start": {
// CHECK-NEXT:                    "line": 14,
// CHECK-NEXT:                    "column": 25
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "end": {
// CHECK-NEXT:                    "line": 14,
// CHECK-NEXT:                    "column": 27
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false,
// CHECK-NEXT:              "loc": {
// CHECK-NEXT:                "start": {
// CHECK-NEXT:                  "line": 14,
// CHECK-NEXT:                  "column": 21
// CHECK-NEXT:                },
// CHECK-NEXT:                "end": {
// CHECK-NEXT:                  "line": 14,
// CHECK-NEXT:                  "column": 27
// CHECK-NEXT:                }
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "set",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "method": false,
// CHECK-NEXT:            "shorthand": false,
// CHECK-NEXT:            "loc": {
// CHECK-NEXT:              "start": {
// CHECK-NEXT:                "line": 14,
// CHECK-NEXT:                "column": 3
// CHECK-NEXT:              },
// CHECK-NEXT:              "end": {
// CHECK-NEXT:                "line": 14,
// CHECK-NEXT:                "column": 27
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ],
// CHECK-NEXT:        "loc": {
// CHECK-NEXT:          "start": {
// CHECK-NEXT:            "line": 11,
// CHECK-NEXT:            "column": 2
// CHECK-NEXT:          },
// CHECK-NEXT:          "end": {
// CHECK-NEXT:            "line": 15,
// CHECK-NEXT:            "column": 2
// CHECK-NEXT:          }
// CHECK-NEXT:        }
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null,
// CHECK-NEXT:      "loc": {
// CHECK-NEXT:        "start": {
// CHECK-NEXT:          "line": 11,
// CHECK-NEXT:          "column": 1
// CHECK-NEXT:        },
// CHECK-NEXT:        "end": {
// CHECK-NEXT:          "line": 15,
// CHECK-NEXT:          "column": 4
// CHECK-NEXT:        }
// CHECK-NEXT:      }
// CHECK-NEXT:    }
// CHECK-NEXT:  ],
// CHECK-NEXT:  "loc": {
// CHECK-NEXT:    "start": {
// CHECK-NEXT:      "line": 11,
// CHECK-NEXT:      "column": 1
// CHECK-NEXT:    },
// CHECK-NEXT:    "end": {
// CHECK-NEXT:      "line": 15,
// CHECK-NEXT:      "column": 4
// CHECK-NEXT:    }
// CHECK-NEXT:  }
// CHECK-NEXT:}
