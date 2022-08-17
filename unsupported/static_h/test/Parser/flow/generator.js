/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

function* foo(x) {
  x ? yield {} : 1;
}

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x"
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExpressionStatement",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "ConditionalExpression",
// CHECK-NEXT:               "test": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "x"
// CHECK-NEXT:               },
// CHECK-NEXT:               "alternate": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 1,
// CHECK-NEXT:                 "raw": "1"
// CHECK-NEXT:               },
// CHECK-NEXT:               "consequent": {
// CHECK-NEXT:                 "type": "YieldExpression",
// CHECK-NEXT:                 "argument": {
// CHECK-NEXT:                   "type": "ObjectExpression",
// CHECK-NEXT:                   "properties": []
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "delegate": false
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "directive": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": true,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
