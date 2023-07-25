/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

if (x) function f() {} else function f() {}

// CHECK:{
// CHECK-NEXT:  "type": "Program",
// CHECK-NEXT:  "body": [
// CHECK-NEXT:    {
// CHECK-NEXT:      "type": "IfStatement",
// CHECK-NEXT:      "test": {
// CHECK-NEXT:        "type": "Identifier",
// CHECK-NEXT:        "name": "x"
// CHECK-NEXT:      },
// CHECK-NEXT:      "consequent": {
// CHECK-NEXT:        "type": "BlockStatement",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "FunctionDeclaration",
// CHECK-NEXT:            "id": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "f"
// CHECK-NEXT:            },
// CHECK-NEXT:            "params": [],
// CHECK-NEXT:            "body": {
// CHECK-NEXT:              "type": "BlockStatement",
// CHECK-NEXT:              "body": []
// CHECK-NEXT:            },
// CHECK-NEXT:            "generator": false,
// CHECK-NEXT:            "async": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      },
// CHECK-NEXT:      "alternate": {
// CHECK-NEXT:        "type": "BlockStatement",
// CHECK-NEXT:        "body": [
// CHECK-NEXT:          {
// CHECK-NEXT:            "type": "FunctionDeclaration",
// CHECK-NEXT:            "id": {
// CHECK-NEXT:              "type": "Identifier",
// CHECK-NEXT:              "name": "f"
// CHECK-NEXT:            },
// CHECK-NEXT:            "params": [],
// CHECK-NEXT:            "body": {
// CHECK-NEXT:              "type": "BlockStatement",
// CHECK-NEXT:              "body": []
// CHECK-NEXT:            },
// CHECK-NEXT:            "generator": false,
// CHECK-NEXT:            "async": false
// CHECK-NEXT:          }
// CHECK-NEXT:        ]
// CHECK-NEXT:      }
// CHECK-NEXT:    }
// CHECK-NEXT:  ]
// CHECK-NEXT:}
