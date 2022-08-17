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

var x = (foo: number): number => 3;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "var",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "ArrowFunctionExpression",
// CHECK-NEXT:             "id": null,
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "foo",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TSTypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSNumberKeyword"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "body": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 3,
// CHECK-NEXT:               "raw": "3"
// CHECK-NEXT:             },
// CHECK-NEXT:             "returnType": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "expression": true,
// CHECK-NEXT:             "async": false
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

var x = (foo: number = 3): number => 3;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "var",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "ArrowFunctionExpression",
// CHECK-NEXT:             "id": null,
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "AssignmentPattern",
// CHECK-NEXT:                 "left": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "foo",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSTypeAnnotation",
// CHECK-NEXT:                     "typeAnnotation": {
// CHECK-NEXT:                       "type": "TSNumberKeyword"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "right": {
// CHECK-NEXT:                   "type": "NumericLiteral",
// CHECK-NEXT:                   "value": 3,
// CHECK-NEXT:                   "raw": "3"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "body": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 3,
// CHECK-NEXT:               "raw": "3"
// CHECK-NEXT:             },
// CHECK-NEXT:             "returnType": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "expression": true,
// CHECK-NEXT:             "async": false
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     },

var x = (foo: number = 3) => 3;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "VariableDeclaration",
// CHECK-NEXT:       "kind": "var",
// CHECK-NEXT:       "declarations": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "VariableDeclarator",
// CHECK-NEXT:           "init": {
// CHECK-NEXT:             "type": "ArrowFunctionExpression",
// CHECK-NEXT:             "id": null,
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "AssignmentPattern",
// CHECK-NEXT:                 "left": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "foo",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TSTypeAnnotation",
// CHECK-NEXT:                     "typeAnnotation": {
// CHECK-NEXT:                       "type": "TSNumberKeyword"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "right": {
// CHECK-NEXT:                   "type": "NumericLiteral",
// CHECK-NEXT:                   "value": 3,
// CHECK-NEXT:                   "raw": "3"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "body": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 3,
// CHECK-NEXT:               "raw": "3"
// CHECK-NEXT:             },
// CHECK-NEXT:             "expression": true,
// CHECK-NEXT:             "async": false
// CHECK-NEXT:           },
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ]
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
