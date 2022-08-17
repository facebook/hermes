/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

(foo) => 1;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 1,
// CHECK-NEXT:           "raw": "1"
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(foo?) => 2;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo",
// CHECK-NEXT:             "optional": true
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 2,
// CHECK-NEXT:           "raw": "2"
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(foo?: number) => 3;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "optional": true
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 3,
// CHECK-NEXT:           "raw": "3"
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(foo? = 1) => 4;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "AssignmentPattern",
// CHECK-NEXT:             "left": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo",
// CHECK-NEXT:               "optional": true
// CHECK-NEXT:             },
// CHECK-NEXT:             "right": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 1,
// CHECK-NEXT:               "raw": "1"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 4,
// CHECK-NEXT:           "raw": "4"
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(foo?: number = 1) => 4;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "AssignmentPattern",
// CHECK-NEXT:             "left": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TypeAnnotation",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 }
// CHECK-NEXT:               },
// CHECK-NEXT:               "optional": true
// CHECK-NEXT:             },
// CHECK-NEXT:             "right": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 1,
// CHECK-NEXT:               "raw": "1"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 4,
// CHECK-NEXT:           "raw": "4"
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(foo: number) => 4;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 4,
// CHECK-NEXT:           "raw": "4"
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": true,
// CHECK-NEXT:         "async": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
