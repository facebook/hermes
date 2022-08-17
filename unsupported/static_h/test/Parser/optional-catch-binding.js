/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

try {
  1;
} catch {
  2;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TryStatement",
// CHECK-NEXT:       "block": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExpressionStatement",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 1,
// CHECK-NEXT:               "raw": "1"
// CHECK-NEXT:             },
// CHECK-NEXT:             "directive": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "handler": {
// CHECK-NEXT:         "type": "CatchClause",
// CHECK-NEXT:         "param": null,
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "ExpressionStatement",
// CHECK-NEXT:               "expression": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 2,
// CHECK-NEXT:                 "raw": "2"
// CHECK-NEXT:               },
// CHECK-NEXT:               "directive": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "finalizer": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
