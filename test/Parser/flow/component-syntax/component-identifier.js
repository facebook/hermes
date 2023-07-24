/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-component-syntax -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

  1 + component;
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
  // CHECK-NEXT:           "type": "Identifier",
  // CHECK-NEXT:           "name": "component"
  // CHECK-NEXT:         },
  // CHECK-NEXT:         "operator": "+"
  // CHECK-NEXT:       },
  // CHECK-NEXT:       "directive": null
  // CHECK-NEXT:     },

  component + 1;
  // CHECK-NEXT:     {
  // CHECK-NEXT:       "type": "ExpressionStatement",
  // CHECK-NEXT:       "expression": {
  // CHECK-NEXT:         "type": "BinaryExpression",
  // CHECK-NEXT:         "left": {
  // CHECK-NEXT:           "type": "Identifier",
  // CHECK-NEXT:           "name": "component"
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

  export default component + 1;
  // CHECK-NEXT:     {
  // CHECK-NEXT:       "type": "ExportDefaultDeclaration",
  // CHECK-NEXT:       "declaration": {
  // CHECK-NEXT:         "type": "BinaryExpression",
  // CHECK-NEXT:         "left": {
  // CHECK-NEXT:           "type": "Identifier",
  // CHECK-NEXT:           "name": "component"
  // CHECK-NEXT:         },
  // CHECK-NEXT:         "right": {
  // CHECK-NEXT:           "type": "NumericLiteral",
  // CHECK-NEXT:           "value": 1,
  // CHECK-NEXT:           "raw": "1"
  // CHECK-NEXT:         },
  // CHECK-NEXT:         "operator": "+"
  // CHECK-NEXT:       }
  // CHECK-NEXT:     }

  // CHECK-NEXT:   ]
  // CHECK-NEXT: }
