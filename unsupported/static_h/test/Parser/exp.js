/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

//CHECK: {
//CHECK-NEXT:    "type": "Program",
//CHECK-NEXT:    "body": [

2 ** 3;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "BinaryExpression",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 2,
// CHECK-NEXT:           "raw": "2"
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 3,
// CHECK-NEXT:           "raw": "3"
// CHECK-NEXT:         },
// CHECK-NEXT:         "operator": "**"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

2 ** 3 ** 4;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "BinaryExpression",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 2,
// CHECK-NEXT:           "raw": "2"
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 3,
// CHECK-NEXT:             "raw": "3"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 4,
// CHECK-NEXT:             "raw": "4"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "**"
// CHECK-NEXT:         },
// CHECK-NEXT:         "operator": "**"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

2 ** 3 ** 4 + 1;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "BinaryExpression",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 2,
// CHECK-NEXT:             "raw": "2"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "BinaryExpression",
// CHECK-NEXT:             "left": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 3,
// CHECK-NEXT:               "raw": "3"
// CHECK-NEXT:             },
// CHECK-NEXT:             "right": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 4,
// CHECK-NEXT:               "raw": "4"
// CHECK-NEXT:             },
// CHECK-NEXT:             "operator": "**"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "**"
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 1,
// CHECK-NEXT:           "raw": "1"
// CHECK-NEXT:         },
// CHECK-NEXT:         "operator": "+"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

1 + 2 ** 3 ** 4;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "BinaryExpression",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 1,
// CHECK-NEXT:           "raw": "1"
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 2,
// CHECK-NEXT:             "raw": "2"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "BinaryExpression",
// CHECK-NEXT:             "left": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 3,
// CHECK-NEXT:               "raw": "3"
// CHECK-NEXT:             },
// CHECK-NEXT:             "right": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 4,
// CHECK-NEXT:               "raw": "4"
// CHECK-NEXT:             },
// CHECK-NEXT:             "operator": "**"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "**"
// CHECK-NEXT:         },
// CHECK-NEXT:         "operator": "+"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

x **= y;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "AssignmentExpression",
// CHECK-NEXT:         "operator": "**=",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x"
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "y"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
