/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -dump-transformed-ast %s | %FileCheckOrRegen --match-full-lines %s

// Test lowering of fastarray push

var x: number[];
x.push(10, 20);

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ExpressionStatement",
// CHECK-NEXT:      "expression": {
// CHECK-NEXT:        "type": "CallExpression",
// CHECK-NEXT:        "callee": {
// CHECK-NEXT:          "type": "FunctionExpression",
// CHECK-NEXT:          "id": null,
// CHECK-NEXT:          "params": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "exports"
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "body": {
// CHECK-NEXT:            "type": "BlockStatement",
// CHECK-NEXT:            "body": [
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "VariableDeclaration",
// CHECK-NEXT:                "kind": "var",
// CHECK-NEXT:                "declarations": [
// CHECK-NEXT:                  {
// CHECK-NEXT:                    "type": "VariableDeclarator",
// CHECK-NEXT:                    "init": null,
// CHECK-NEXT:                    "id": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "x",
// CHECK-NEXT:                      "typeAnnotation": {
// CHECK-NEXT:                        "type": "TypeAnnotation",
// CHECK-NEXT:                        "typeAnnotation": {
// CHECK-NEXT:                          "type": "ArrayTypeAnnotation",
// CHECK-NEXT:                          "elementType": {
// CHECK-NEXT:                            "type": "NumberTypeAnnotation"
// CHECK-NEXT:                          }
// CHECK-NEXT:                        }
// CHECK-NEXT:                      }
// CHECK-NEXT:                    }
// CHECK-NEXT:                  }
// CHECK-NEXT:                ]
// CHECK-NEXT:              },
// CHECK-NEXT:              {
// CHECK-NEXT:                "type": "ExpressionStatement",
// CHECK-NEXT:                "expression": {
// CHECK-NEXT:                  "type": "CallExpression",
// CHECK-NEXT:                  "callee": {
// CHECK-NEXT:                    "type": "MemberExpression",
// CHECK-NEXT:                    "object": {
// CHECK-NEXT:                      "type": "SHBuiltin"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "property": {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "?fastArrayPush"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    "computed": false
// CHECK-NEXT:                  },
// CHECK-NEXT:                  "arguments": [
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "Identifier",
// CHECK-NEXT:                      "name": "x"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "NumericLiteral",
// CHECK-NEXT:                      "value": 10,
// CHECK-NEXT:                      "raw": "10"
// CHECK-NEXT:                    },
// CHECK-NEXT:                    {
// CHECK-NEXT:                      "type": "NumericLiteral",
// CHECK-NEXT:                      "value": 20,
// CHECK-NEXT:                      "raw": "20"
// CHECK-NEXT:                    }
// CHECK-NEXT:                  ]
// CHECK-NEXT:                },
// CHECK-NEXT:                "directive": null
// CHECK-NEXT:              }
// CHECK-NEXT:            ]
// CHECK-NEXT:          },
// CHECK-NEXT:          "generator": false,
// CHECK-NEXT:          "async": false
// CHECK-NEXT:        },
// CHECK-NEXT:        "arguments": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "ObjectExpression",
// CHECK-NEXT:            "properties": []
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      },
// CHECK-NEXT:      "directive": null
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
