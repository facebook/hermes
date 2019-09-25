// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermesc -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

var tmp = { a : 1, b, c };
//CHECK:              "type": "VariableDeclaration",
//CHECK-NEXT:         "kind": "var",
//CHECK-NEXT:         "declarations": [
//CHECK-NEXT:           {
//CHECK-NEXT:             "type": "VariableDeclarator",
//CHECK-NEXT:             "init": {
//CHECK-NEXT:               "type": "ObjectExpression",
//CHECK-NEXT:               "properties": [
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "a",
//CHECK-NEXT:                     "typeAnnotation": null
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "NumericLiteral",
//CHECK-NEXT:                     "value": 1,
//CHECK-NEXT:                     "raw": "1"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "b",
//CHECK-NEXT:                     "typeAnnotation": null
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "b",
//CHECK-NEXT:                     "typeAnnotation": null
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "c",
//CHECK-NEXT:                     "typeAnnotation": null
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "c",
//CHECK-NEXT:                     "typeAnnotation": null
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "tmp",
//CHECK-NEXT:               "typeAnnotation": null
//CHECK-NEXT:             }
//CHECK-NEXT:           }
//CHECK-NEXT:         ]

var tmp1 = { d };
//CHECK:             "type": "VariableDeclaration",
//CHECK-NEXT:        "kind": "var",
//CHECK-NEXT:        "declarations": [
//CHECK-NEXT:          {
//CHECK-NEXT:            "type": "VariableDeclarator",
//CHECK-NEXT:            "init": {
//CHECK-NEXT:              "type": "ObjectExpression",
//CHECK-NEXT:              "properties": [
//CHECK-NEXT:                {
//CHECK-NEXT:                  "type": "Property",
//CHECK-NEXT:                  "key": {
//CHECK-NEXT:                    "type": "Identifier",
//CHECK-NEXT:                    "name": "d",
//CHECK-NEXT:                    "typeAnnotation": null
//CHECK-NEXT:                  },
//CHECK-NEXT:                  "value": {
//CHECK-NEXT:                    "type": "Identifier",
//CHECK-NEXT:                    "name": "d",
//CHECK-NEXT:                    "typeAnnotation": null
//CHECK-NEXT:                  },
//CHECK-NEXT:                  "kind": "init",
//CHECK-NEXT:                  "computed": false
//CHECK-NEXT:                }
//CHECK-NEXT:              ]
//CHECK-NEXT:            },
//CHECK-NEXT:            "id": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "tmp1",
//CHECK-NEXT:              "typeAnnotation": null
//CHECK-NEXT:            }
//CHECK-NEXT:          }
//CHECK-NEXT:        ]
