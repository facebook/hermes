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

import('foo')
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ImportExpression",
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "attributes": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

1 + import('foo')
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
// CHECK-NEXT:           "type": "ImportExpression",
// CHECK-NEXT:           "source": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "attributes": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "operator": "+"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

function func() {
  import('foo').then();
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "func"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExpressionStatement",
// CHECK-NEXT:             "expression": {
// CHECK-NEXT:               "type": "CallExpression",
// CHECK-NEXT:               "callee": {
// CHECK-NEXT:                 "type": "MemberExpression",
// CHECK-NEXT:                 "object": {
// CHECK-NEXT:                   "type": "ImportExpression",
// CHECK-NEXT:                   "source": {
// CHECK-NEXT:                     "type": "StringLiteral",
// CHECK-NEXT:                     "value": "foo"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "attributes": null
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "property": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "then"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "computed": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "arguments": []
// CHECK-NEXT:             },
// CHECK-NEXT:             "directive": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
