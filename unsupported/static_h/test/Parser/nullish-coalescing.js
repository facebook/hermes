/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

a ?? b;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "LogicalExpression",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "a"
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "b"
// CHECK-NEXT:         },
// CHECK-NEXT:         "operator": "??"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

a ?? (b && c);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "LogicalExpression",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "a"
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "LogicalExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "b"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "c"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "&&"
// CHECK-NEXT:         },
// CHECK-NEXT:         "operator": "??"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
