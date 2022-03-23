/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines
// RUN: %hermes -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

foo < bar;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "BinaryExpression",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "bar"
// CHECK-NEXT:         },
// CHECK-NEXT:         "operator": "<"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

foo < bar && bar > baz;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "LogicalExpression",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "bar"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "<"
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "bar"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "baz"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": ">"
// CHECK-NEXT:         },
// CHECK-NEXT:         "operator": "&&"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
