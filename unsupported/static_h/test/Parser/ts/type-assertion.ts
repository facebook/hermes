/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-ts -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

<T>x;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "TSTypeAssertion",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TSTypeReference",
// CHECK-NEXT:           "typeName": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "T"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

<T><U>x;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "TSTypeAssertion",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TSTypeReference",
// CHECK-NEXT:           "typeName": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "T"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": {
// CHECK-NEXT:           "type": "TSTypeAssertion",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "TSTypeReference",
// CHECK-NEXT:             "typeName": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "U"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "expression": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

<T>++x;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "TSTypeAssertion",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TSTypeReference",
// CHECK-NEXT:           "typeName": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "T"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": {
// CHECK-NEXT:           "type": "UpdateExpression",
// CHECK-NEXT:           "operator": "++",
// CHECK-NEXT:           "argument": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "prefix": true
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
