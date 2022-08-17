/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-ts -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines
// RUN: %hermes -parse-jsx -parse-ts -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

let x: number = 3;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "let",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 3,
// CHECK-NEXT:             "raw": "3"
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

let [x: number] = 3;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "let",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 3,
// CHECK-NEXT:             "raw": "3"
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "ArrayPattern",
// CHECK-NEXT:             "elements": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "x",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSTypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

let {x}: T = 3;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "let",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 3,
// CHECK-NEXT:             "raw": "3"
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "ObjectPattern",
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
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSTypeReference",
// CHECK-NEXT:                 "typeName": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "T"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
