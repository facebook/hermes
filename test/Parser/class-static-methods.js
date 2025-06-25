/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc %s -dump-ast | %FileCheckOrRegen --match-full-lines %s

class A {
  static #prototype() {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "ClassDeclaration",
// CHECK-NEXT:      "id": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "A"
// CHECK-NEXT:      },
// CHECK-NEXT:      "superClass": null,
// CHECK-NEXT:      "body": {
// CHECK-NEXT:        "type": "ClassBody",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "MethodDefinition",
// CHECK-NEXT:            "key": {
// CHECK-NEXT:              "type": "PrivateName",
// CHECK-NEXT:              "id": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "prototype"
// CHECK-NEXT:              }
// CHECK-NEXT:            },
// CHECK-NEXT:            "value": {
// CHECK-NEXT:              "type": "FunctionExpression",
// CHECK-NEXT:              "id": null,
// CHECK-NEXT:              "params": [],
// CHECK-NEXT:              "body": {
// CHECK-NEXT:                "type": "BlockStatement",
// CHECK-NEXT:                "body": []
// CHECK-NEXT:              },
// CHECK-NEXT:              "generator": false,
// CHECK-NEXT:              "async": false
// CHECK-NEXT:            },
// CHECK-NEXT:            "kind": "method",
// CHECK-NEXT:            "computed": false,
// CHECK-NEXT:            "static": true
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
