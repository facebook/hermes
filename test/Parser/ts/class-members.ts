/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-ts -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [

// Field declarations
class Point {
  x: number;
  y: number;
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Point"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "x"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": null,
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "y"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": null,
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

// Field declarations with initializers
class Point {
  x = 0;
  y = 0;
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Point"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "x"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 0,
// CHECK-NEXT:              "raw": "0"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "y"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 0,
// CHECK-NEXT:              "raw": "0"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

// Constructor
class Point {
  x: number;
  y: number;

  constructor() {
    this.x = 0;
    this.y = 0;
  }
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Point"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "x"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": null,
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "y"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": null,
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "MethodDefinition",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "constructor"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ExpressionStatement",
// CHECK-NEXT:                    "expression": {
// CHECK-NEXT:                      "type": "AssignmentExpression",
// CHECK-NEXT:                      "operator": "=",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "MemberExpression",
// CHECK-NEXT:                        "object": {
// CHECK-NEXT:                          "type": "ThisExpression"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "property": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "x"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "computed": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 0,
// CHECK-NEXT:                        "raw": "0"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "directive": null
// CHECK-NEXT:                  },
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ExpressionStatement",
// CHECK-NEXT:                    "expression": {
// CHECK-NEXT:                      "type": "AssignmentExpression",
// CHECK-NEXT:                      "operator": "=",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "MemberExpression",
// CHECK-NEXT:                        "object": {
// CHECK-NEXT:                          "type": "ThisExpression"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "property": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "y"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "computed": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "NumericLiteral",
// CHECK-NEXT:                        "value": 0,
// CHECK-NEXT:                        "raw": "0"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "directive": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "constructor",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

// Constructor with parameters
class Point {
  x: number;
  y: number;

  constructor(x: number, y: number) {
    this.x = x;
    this.y = y;
  }
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Point"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "x"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": null,
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "y"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": null,
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "MethodDefinition",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "constructor"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "x",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "TSTypeAnnotation",
// CHECK-NEXT:                    "typeAnnotation": {
// CHECK-NEXT:                      "type": "TSNumberKeyword"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "y",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "TSTypeAnnotation",
// CHECK-NEXT:                    "typeAnnotation": {
// CHECK-NEXT:                      "type": "TSNumberKeyword"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              ],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ExpressionStatement",
// CHECK-NEXT:                    "expression": {
// CHECK-NEXT:                      "type": "AssignmentExpression",
// CHECK-NEXT:                      "operator": "=",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "MemberExpression",
// CHECK-NEXT:                        "object": {
// CHECK-NEXT:                          "type": "ThisExpression"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "property": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "x"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "computed": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "x"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "directive": null
// CHECK-NEXT:                  },
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ExpressionStatement",
// CHECK-NEXT:                    "expression": {
// CHECK-NEXT:                      "type": "AssignmentExpression",
// CHECK-NEXT:                      "operator": "=",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "MemberExpression",
// CHECK-NEXT:                        "object": {
// CHECK-NEXT:                          "type": "ThisExpression"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "property": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "y"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "computed": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "y"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "directive": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "constructor",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

// Constructor with parameters with default values
class Point {
  x: number;
  y: number;

  constructor(x = 0, y = 0) {
    this.x = x;
    this.y = y;
  }
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Point"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "x"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": null,
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "y"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": null,
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "MethodDefinition",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "constructor"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "AssignmentPattern",
// CHECK-NEXT:                  "left": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "x"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "right": {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 0,
// CHECK-NEXT:                    "raw": "0"
// CHECK-NEXT:                  }
// CHECK-NEXT:                },
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "AssignmentPattern",
// CHECK-NEXT:                  "left": {
// CHECK-NEXT:                    "type": "Identifier",
// CHECK-NEXT:                    "name": "y"
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "right": {
// CHECK-NEXT:                    "type": "NumericLiteral",
// CHECK-NEXT:                    "value": 0,
// CHECK-NEXT:                    "raw": "0"
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              ],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ExpressionStatement",
// CHECK-NEXT:                    "expression": {
// CHECK-NEXT:                      "type": "AssignmentExpression",
// CHECK-NEXT:                      "operator": "=",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "MemberExpression",
// CHECK-NEXT:                        "object": {
// CHECK-NEXT:                          "type": "ThisExpression"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "property": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "x"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "computed": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "x"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "directive": null
// CHECK-NEXT:                  },
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ExpressionStatement",
// CHECK-NEXT:                    "expression": {
// CHECK-NEXT:                      "type": "AssignmentExpression",
// CHECK-NEXT:                      "operator": "=",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "MemberExpression",
// CHECK-NEXT:                        "object": {
// CHECK-NEXT:                          "type": "ThisExpression"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "property": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "y"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "computed": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "y"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "directive": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "constructor",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

// Method declaration
class Point {
  x = 10;
  y = 10;

  scale(n: number): void {
    this.x *= n;
    this.y *= n;
  }
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Point"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "x"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 10,
// CHECK-NEXT:              "raw": "10"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "y"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 10,
// CHECK-NEXT:              "raw": "10"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "MethodDefinition",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "scale"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "n",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "TSTypeAnnotation",
// CHECK-NEXT:                    "typeAnnotation": {
// CHECK-NEXT:                      "type": "TSNumberKeyword"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              ],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ExpressionStatement",
// CHECK-NEXT:                    "expression": {
// CHECK-NEXT:                      "type": "AssignmentExpression",
// CHECK-NEXT:                      "operator": "*=",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "MemberExpression",
// CHECK-NEXT:                        "object": {
// CHECK-NEXT:                          "type": "ThisExpression"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "property": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "x"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "computed": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "n"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "directive": null
// CHECK-NEXT:                  },
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ExpressionStatement",
// CHECK-NEXT:                    "expression": {
// CHECK-NEXT:                      "type": "AssignmentExpression",
// CHECK-NEXT:                      "operator": "*=",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "MemberExpression",
// CHECK-NEXT:                        "object": {
// CHECK-NEXT:                          "type": "ThisExpression"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "property": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "y"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "computed": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "n"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "directive": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              "returnType": {
// CHECK-NEXT:                "type": "TSTypeAnnotation",
// CHECK-NEXT:                "typeAnnotation": {
// CHECK-NEXT:                  "type": "TSVoidKeyword"
// CHECK-NEXT:                }
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "method",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

// Getter and Setter
class Thing {
  _name = "";
  get name(): string {
    return this._name;
  }
  set name(name: string) {
    this._name = name;
  }
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Thing"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "_name"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "StringLiteral",
// CHECK-NEXT:              "value": ""
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "MethodDefinition",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "name"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ReturnStatement",
// CHECK-NEXT:                    "argument": {
// CHECK-NEXT:                      "type": "MemberExpression",
// CHECK-NEXT:                      "object": {
// CHECK-NEXT:                        "type": "ThisExpression"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "property": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "_name"
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "computed": false
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              "returnType": {
// CHECK-NEXT:                "type": "TSTypeAnnotation",
// CHECK-NEXT:                "typeAnnotation": {
// CHECK-NEXT:                  "type": "TSStringKeyword"
// CHECK-NEXT:                }
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "get",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "MethodDefinition",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "name"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "Identifier",
// CHECK-NEXT:                  "name": "name",
// CHECK-NEXT:                  "typeAnnotation": {
// CHECK-NEXT:                    "type": "TSTypeAnnotation",
// CHECK-NEXT:                    "typeAnnotation": {
// CHECK-NEXT:                      "type": "TSStringKeyword"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                }
// CHECK-NEXT:              ],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "ExpressionStatement",
// CHECK-NEXT:                    "expression": {
// CHECK-NEXT:                      "type": "AssignmentExpression",
// CHECK-NEXT:                      "operator": "=",
// CHECK-NEXT:                      "left": {
// CHECK-NEXT:                        "type": "MemberExpression",
// CHECK-NEXT:                        "object": {
// CHECK-NEXT:                          "type": "ThisExpression"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "property": {
// CHECK-NEXT:                          "type": "Identifier",
// CHECK-NEXT:                          "name": "_name"
// CHECK-NEXT:                        },
// CHECK-NEXT:                        "computed": false
// CHECK-NEXT:                      },
// CHECK-NEXT:                      "right": {
// CHECK-NEXT:                        "type": "Identifier",
// CHECK-NEXT:                        "name": "name"
// CHECK-NEXT:                      }
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "directive": null
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "set",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    }

// CHECK-NEXT:  ]
// CHECK-NEXT:}
