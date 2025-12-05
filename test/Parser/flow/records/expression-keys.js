/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-flow-records -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

const x = R {"key": 1, "another-key": 2};

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "const",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "RecordExpression",
// CHECK-NEXT:             "constructor": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "R"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeArguments": null,
// CHECK-NEXT:             "properties": {
// CHECK-NEXT:               "type": "RecordExpressionProperties",
// CHECK-NEXT:               "properties": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Property",
// CHECK-NEXT:                   "key": {
// CHECK-NEXT:                     "type": "StringLiteral",
// CHECK-NEXT:                     "value": "key"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "value": {
// CHECK-NEXT:                     "type": "NumericLiteral",
// CHECK-NEXT:                     "value": 1,
// CHECK-NEXT:                     "raw": "1"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "kind": "init",
// CHECK-NEXT:                   "computed": false,
// CHECK-NEXT:                   "method": false,
// CHECK-NEXT:                   "shorthand": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Property",
// CHECK-NEXT:                   "key": {
// CHECK-NEXT:                     "type": "StringLiteral",
// CHECK-NEXT:                     "value": "another-key"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "value": {
// CHECK-NEXT:                     "type": "NumericLiteral",
// CHECK-NEXT:                     "value": 2,
// CHECK-NEXT:                     "raw": "2"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "kind": "init",
// CHECK-NEXT:                   "computed": false,
// CHECK-NEXT:                   "method": false,
// CHECK-NEXT:                   "shorthand": false
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

const y = R {1: "one", 2: "two", 3: "three"};

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "const",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "RecordExpression",
// CHECK-NEXT:             "constructor": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "R"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeArguments": null,
// CHECK-NEXT:             "properties": {
// CHECK-NEXT:               "type": "RecordExpressionProperties",
// CHECK-NEXT:               "properties": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Property",
// CHECK-NEXT:                   "key": {
// CHECK-NEXT:                     "type": "NumericLiteral",
// CHECK-NEXT:                     "value": 1,
// CHECK-NEXT:                     "raw": "1"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "value": {
// CHECK-NEXT:                     "type": "StringLiteral",
// CHECK-NEXT:                     "value": "one"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "kind": "init",
// CHECK-NEXT:                   "computed": false,
// CHECK-NEXT:                   "method": false,
// CHECK-NEXT:                   "shorthand": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Property",
// CHECK-NEXT:                   "key": {
// CHECK-NEXT:                     "type": "NumericLiteral",
// CHECK-NEXT:                     "value": 2,
// CHECK-NEXT:                     "raw": "2"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "value": {
// CHECK-NEXT:                     "type": "StringLiteral",
// CHECK-NEXT:                     "value": "two"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "kind": "init",
// CHECK-NEXT:                   "computed": false,
// CHECK-NEXT:                   "method": false,
// CHECK-NEXT:                   "shorthand": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Property",
// CHECK-NEXT:                   "key": {
// CHECK-NEXT:                     "type": "NumericLiteral",
// CHECK-NEXT:                     "value": 3,
// CHECK-NEXT:                     "raw": "3"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "value": {
// CHECK-NEXT:                     "type": "StringLiteral",
// CHECK-NEXT:                     "value": "three"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "kind": "init",
// CHECK-NEXT:                   "computed": false,
// CHECK-NEXT:                   "method": false,
// CHECK-NEXT:                   "shorthand": false
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "y"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
