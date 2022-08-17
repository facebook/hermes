/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL: {
// CHECK-NEXT:    "type": "Program",
// CHECK-NEXT:    "body": [

`abc`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "abc",
// CHECK-NEXT:              "raw": "abc"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

`abc\ndef`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "abc\ndef",
// CHECK-NEXT:              "raw": "abc\\ndef"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

`abc\`def`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "abc`def",
// CHECK-NEXT:              "raw": "abc\\`def"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

`a${`x${3}y`}b`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": false,
// CHECK-NEXT:              "cooked": "a",
// CHECK-NEXT:              "raw": "a"
// CHECK-NEXT:            },
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "b",
// CHECK-NEXT:              "raw": "b"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateLiteral",
// CHECK-NEXT:              "quasis": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "TemplateElement",
// CHECK-NEXT:                  "tail": false,
// CHECK-NEXT:                  "cooked": "x",
// CHECK-NEXT:                  "raw": "x"
// CHECK-NEXT:                },
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "TemplateElement",
// CHECK-NEXT:                  "tail": true,
// CHECK-NEXT:                  "cooked": "y",
// CHECK-NEXT:                  "raw": "y"
// CHECK-NEXT:                }
// CHECK-NEXT:              ],
// CHECK-NEXT:              "expressions": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "NumericLiteral",
// CHECK-NEXT:                  "value": 3,
// CHECK-NEXT:                  "raw": "3"
// CHECK-NEXT:                }
// CHECK-NEXT:              ]
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

`a \n \r \r\n b`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "a \n \r \r\n b",
// CHECK-NEXT:              "raw": "a \\n \\r \\r\\n b"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

`abc\
def`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "abcdef",
// CHECK-NEXT:              "raw": "abc\\\ndef"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

// Test <CR>
`abc\def`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "abcdef",
// CHECK-NEXT:              "raw": "abc\\\ndef"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

`abcdef`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "abc\ndef",
// CHECK-NEXT:              "raw": "abc\ndef"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

// Test <CR><LF>
`abc\
def`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "abcdef",
// CHECK-NEXT:              "raw": "abc\\\ndef"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

`abc${x}def`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": false,
// CHECK-NEXT:              "cooked": "abc",
// CHECK-NEXT:              "raw": "abc"
// CHECK-NEXT:            },
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "def",
// CHECK-NEXT:              "raw": "def"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "x"
// CHECK-NEXT:            }
// CHECK-NEXT:          ]
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

tag`abc`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TaggedTemplateExpression",
// CHECK-NEXT:          "tag": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "tag"
// CHECK-NEXT:          },
// CHECK-NEXT:          "quasi": {
// CHECK-NEXT:            "type": "TemplateLiteral",
// CHECK-NEXT:            "quasis": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "TemplateElement",
// CHECK-NEXT:                "tail": true,
// CHECK-NEXT:                "cooked": "abc",
// CHECK-NEXT:                "raw": "abc"
// CHECK-NEXT:              }
// CHECK-NEXT:            ],
// CHECK-NEXT:            "expressions": []
// CHECK-NEXT:          }
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

new tag`abc`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "NewExpression",
// CHECK-NEXT:          "callee": {
// CHECK-NEXT:            "type": "TaggedTemplateExpression",
// CHECK-NEXT:            "tag": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "tag"
// CHECK-NEXT:            },
// CHECK-NEXT:            "quasi": {
// CHECK-NEXT:              "type": "TemplateLiteral",
// CHECK-NEXT:              "quasis": [
// CHECK-NEXT:                {
// CHECK-NEXT:                  "type": "TemplateElement",
// CHECK-NEXT:                  "tail": true,
// CHECK-NEXT:                  "cooked": "abc",
// CHECK-NEXT:                  "raw": "abc"
// CHECK-NEXT:                }
// CHECK-NEXT:              ],
// CHECK-NEXT:              "expressions": []
// CHECK-NEXT:            }
// CHECK-NEXT:          },
// CHECK-NEXT:          "arguments": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

`\x41\x42`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TemplateLiteral",
// CHECK-NEXT:          "quasis": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "TemplateElement",
// CHECK-NEXT:              "tail": true,
// CHECK-NEXT:              "cooked": "AB",
// CHECK-NEXT:              "raw": "\\x41\\x42"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "expressions": []
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

tag`\xyz`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TaggedTemplateExpression",
// CHECK-NEXT:          "tag": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "tag"
// CHECK-NEXT:          },
// CHECK-NEXT:          "quasi": {
// CHECK-NEXT:            "type": "TemplateLiteral",
// CHECK-NEXT:            "quasis": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "TemplateElement",
// CHECK-NEXT:                "tail": true,
// CHECK-NEXT:                "cooked": null,
// CHECK-NEXT:                "raw": "\\xyz"
// CHECK-NEXT:              }
// CHECK-NEXT:            ],
// CHECK-NEXT:            "expressions": []
// CHECK-NEXT:          }
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

tag`\unicode`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TaggedTemplateExpression",
// CHECK-NEXT:          "tag": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "tag"
// CHECK-NEXT:          },
// CHECK-NEXT:          "quasi": {
// CHECK-NEXT:            "type": "TemplateLiteral",
// CHECK-NEXT:            "quasis": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "TemplateElement",
// CHECK-NEXT:                "tail": true,
// CHECK-NEXT:                "cooked": null,
// CHECK-NEXT:                "raw": "\\unicode"
// CHECK-NEXT:              }
// CHECK-NEXT:            ],
// CHECK-NEXT:            "expressions": []
// CHECK-NEXT:          }
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      },

tag`脸\书𐀀\𐀁`;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "ExpressionStatement",
// CHECK-NEXT:        "expression": {
// CHECK-NEXT:          "type": "TaggedTemplateExpression",
// CHECK-NEXT:          "tag": {
// CHECK-NEXT:            "type": "Identifier",
// CHECK-NEXT:            "name": "tag"
// CHECK-NEXT:          },
// CHECK-NEXT:          "quasi": {
// CHECK-NEXT:            "type": "TemplateLiteral",
// CHECK-NEXT:            "quasis": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "TemplateElement",
// CHECK-NEXT:                "tail": true,
// CHECK-NEXT:                "cooked": "\u8138\u4e66\ud800\udc00\ud800\udc01",
// CHECK-NEXT:                "raw": "\u8138\\\u4e66\ud800\udc00\\\ud800\udc01"
// CHECK-NEXT:              }
// CHECK-NEXT:            ],
// CHECK-NEXT:            "expressions": []
// CHECK-NEXT:          }
// CHECK-NEXT:        },
// CHECK-NEXT:        "directive": null
// CHECK-NEXT:      }

// CHECK-NEXT:    ]
// CHECK-NEXT: }
