/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -parse-jsx -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

x ? (1) : <foo>"</foo>;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ConditionalExpression",
// CHECK-NEXT:         "test": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x"
// CHECK-NEXT:         },
// CHECK-NEXT:         "alternate": {
// CHECK-NEXT:           "type": "JSXElement",
// CHECK-NEXT:           "openingElement": {
// CHECK-NEXT:             "type": "JSXOpeningElement",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "JSXIdentifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "attributes": [],
// CHECK-NEXT:             "selfClosing": false
// CHECK-NEXT:           },
// CHECK-NEXT:           "children": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "JSXText",
// CHECK-NEXT:               "value": "\"",
// CHECK-NEXT:               "raw": "\""
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "closingElement": {
// CHECK-NEXT:             "type": "JSXClosingElement",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "JSXIdentifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "consequent": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 1,
// CHECK-NEXT:           "raw": "1"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

x ? (1) : <foo>#{bar}</foo>;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ConditionalExpression",
// CHECK-NEXT:         "test": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "x"
// CHECK-NEXT:         },
// CHECK-NEXT:         "alternate": {
// CHECK-NEXT:           "type": "JSXElement",
// CHECK-NEXT:           "openingElement": {
// CHECK-NEXT:             "type": "JSXOpeningElement",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "JSXIdentifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "attributes": [],
// CHECK-NEXT:             "selfClosing": false
// CHECK-NEXT:           },
// CHECK-NEXT:           "children": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "JSXText",
// CHECK-NEXT:               "value": "#",
// CHECK-NEXT:               "raw": "#"
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "JSXExpressionContainer",
// CHECK-NEXT:               "expression": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "bar"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "closingElement": {
// CHECK-NEXT:             "type": "JSXClosingElement",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "JSXIdentifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "consequent": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 1,
// CHECK-NEXT:           "raw": "1"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
