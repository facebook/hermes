/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

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
//CHECK-NEXT:                     "name": "a"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "NumericLiteral",
//CHECK-NEXT:                     "value": 1,
//CHECK-NEXT:                     "raw": "1"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": false
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "b"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "b"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": true
//CHECK-NEXT:                 },
//CHECK-NEXT:                 {
//CHECK-NEXT:                   "type": "Property",
//CHECK-NEXT:                   "key": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "c"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "value": {
//CHECK-NEXT:                     "type": "Identifier",
//CHECK-NEXT:                     "name": "c"
//CHECK-NEXT:                   },
//CHECK-NEXT:                   "kind": "init",
//CHECK-NEXT:                   "computed": false,
//CHECK-NEXT:                   "method": false,
//CHECK-NEXT:                   "shorthand": true
//CHECK-NEXT:                 }
//CHECK-NEXT:               ]
//CHECK-NEXT:             },
//CHECK-NEXT:             "id": {
//CHECK-NEXT:               "type": "Identifier",
//CHECK-NEXT:               "name": "tmp"
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
//CHECK-NEXT:                    "name": "d"
//CHECK-NEXT:                  },
//CHECK-NEXT:                  "value": {
//CHECK-NEXT:                    "type": "Identifier",
//CHECK-NEXT:                    "name": "d"
//CHECK-NEXT:                  },
//CHECK-NEXT:                  "kind": "init",
//CHECK-NEXT:                  "computed": false,
//CHECK-NEXT:                  "method": false,
//CHECK-NEXT:                  "shorthand": true
//CHECK-NEXT:                }
//CHECK-NEXT:              ]
//CHECK-NEXT:            },
//CHECK-NEXT:            "id": {
//CHECK-NEXT:              "type": "Identifier",
//CHECK-NEXT:              "name": "tmp1"
//CHECK-NEXT:            }
//CHECK-NEXT:          }
//CHECK-NEXT:        ]

var a1 = { async, x };

// CHECK:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "var",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "ObjectExpression",
// CHECK-NEXT:             "properties": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": true
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": true
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "a1"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]

var a2 = { x, async };

// CHECK:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "var",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "ObjectExpression",
// CHECK-NEXT:             "properties": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": true
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "async"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": true
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "a2"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]

var getset = {
  get,
  set,
};
// CHECK:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "var",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "ObjectExpression",
// CHECK-NEXT:             "properties": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "get"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "get"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": true
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "set"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "set"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": true
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "getset"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
