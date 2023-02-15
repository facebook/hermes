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

class Static {
  static: number = 42;
  static static: string = "hello";
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Static"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "static"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 42,
// CHECK-NEXT:              "raw": "42"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "static"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "StringLiteral",
// CHECK-NEXT:              "value": "hello"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSStringKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

class ReadOnly {
  readonly: number = 42;
  readonly static: string = "hello";
  static readonly readonly: number = 123;
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "ReadOnly"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "readonly"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 42,
// CHECK-NEXT:              "raw": "42"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "static"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "StringLiteral",
// CHECK-NEXT:              "value": "hello"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSStringKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "readonly"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 123,
// CHECK-NEXT:              "raw": "123"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

class Private {
  private: number = 42;
  private str: string = "hello";
  private readonly val: number = 123;
  private static private: number = 456;
  private static readonly static: number = 789;
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Private"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "private"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 42,
// CHECK-NEXT:              "raw": "42"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "str"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "StringLiteral",
// CHECK-NEXT:              "value": "hello"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSStringKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "private",
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "val"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 123,
// CHECK-NEXT:              "raw": "123"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "private",
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "private"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 456,
// CHECK-NEXT:              "raw": "456"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "private",
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "static"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 789,
// CHECK-NEXT:              "raw": "789"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "private",
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

class Protected {
  protected: number = 42;
  protected str: string = "hello";
  protected readonly val: number = 123;
  protected static static: number = 456;
  protected static readonly protected: number = 789;
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Protected"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "protected"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 42,
// CHECK-NEXT:              "raw": "42"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "str"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "StringLiteral",
// CHECK-NEXT:              "value": "hello"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSStringKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "protected",
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "val"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 123,
// CHECK-NEXT:              "raw": "123"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "protected",
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "static"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 456,
// CHECK-NEXT:              "raw": "456"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "protected",
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "protected"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 789,
// CHECK-NEXT:              "raw": "789"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "protected",
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

class Public {
  public: number = 42;
  public str: string = "hello";
  public readonly val: number = 123;
  public static public: number = 456;
  public static readonly static: number = 789;
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "Public"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "public"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 42,
// CHECK-NEXT:              "raw": "42"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "str"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "StringLiteral",
// CHECK-NEXT:              "value": "hello"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSStringKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "public",
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "val"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 123,
// CHECK-NEXT:              "raw": "123"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "public",
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "public"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 456,
// CHECK-NEXT:              "raw": "456"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "public",
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "static"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 789,
// CHECK-NEXT:              "raw": "789"
// CHECK-NEXT:            },
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "typeAnnotation": {
// CHECK-NEXT:              "type": "TSTypeAnnotation",
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "TSNumberKeyword"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": "public",
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    },

class HashPrivate {
  #foo = 1;
  readonly #fooRO = 2;
  static #bar = 3;
  static readonly #barRO = 4;
}
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "HashPrivate"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassPrivateProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "foo"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 1,
// CHECK-NEXT:              "raw": "1"
// CHECK-NEXT:            },
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassPrivateProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "fooRO"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 2,
// CHECK-NEXT:              "raw": "2"
// CHECK-NEXT:            },
// CHECK-NEXT:            "static": false,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassPrivateProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "bar"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 3,
// CHECK-NEXT:              "raw": "3"
// CHECK-NEXT:            },
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": false
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ClassPrivateProperty",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "barRO"
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "NumericLiteral",
// CHECK-NEXT:              "value": 4,
// CHECK-NEXT:              "raw": "4"
// CHECK-NEXT:            },
// CHECK-NEXT:            "static": true,
// CHECK-NEXT:            "declare": false,
// CHECK-NEXT:            "tsModifiers": {
// CHECK-NEXT:              "type": "TSModifiers",
// CHECK-NEXT:              "accessibility": null,
// CHECK-NEXT:              "readonly": true
// CHECK-NEXT:            }
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    }

// CHECK-NEXT:  ]
// CHECK-NEXT:}
