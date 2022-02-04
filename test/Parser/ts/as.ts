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

x as number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "TSAsExpression",
// CHECK-NEXT:         "expression": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TSNumberKeyword"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

x as const;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "TSAsExpression",
// CHECK-NEXT:         "expression": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TSTypeReference",
// CHECK-NEXT:           "typeName": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "const"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

x + y as number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "TSAsExpression",
// CHECK-NEXT:         "expression": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "y"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "+"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "TSNumberKeyword"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
