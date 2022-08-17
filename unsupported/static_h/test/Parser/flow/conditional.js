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

foo ? bar : baz;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ConditionalExpression",
// CHECK-NEXT:         "test": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "alternate": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "baz"
// CHECK-NEXT:         },
// CHECK-NEXT:         "consequent": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "bar"
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

(foo ? () => bar : baz);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ConditionalExpression",
// CHECK-NEXT:         "test": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "alternate": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "baz"
// CHECK-NEXT:         },
// CHECK-NEXT:         "consequent": {
// CHECK-NEXT:           "type": "ArrowFunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "bar"
// CHECK-NEXT:           },
// CHECK-NEXT:           "expression": true,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

foo ? bar1 => (baz1) : bar2 => (baz2);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ConditionalExpression",
// CHECK-NEXT:         "test": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "alternate": {
// CHECK-NEXT:           "type": "ArrowFunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar2"
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "baz2"
// CHECK-NEXT:           },
// CHECK-NEXT:           "expression": true,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "consequent": {
// CHECK-NEXT:           "type": "ArrowFunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar1"
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "baz1"
// CHECK-NEXT:           },
// CHECK-NEXT:           "expression": true,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

foo ? (bar1 => (baz1)) : bar2 => (baz2);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ConditionalExpression",
// CHECK-NEXT:         "test": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "alternate": {
// CHECK-NEXT:           "type": "ArrowFunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar2"
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "baz2"
// CHECK-NEXT:           },
// CHECK-NEXT:           "expression": true,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         },
// CHECK-NEXT:         "consequent": {
// CHECK-NEXT:           "type": "ArrowFunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar1"
// CHECK-NEXT:             }
// CHECK-NEXT:           ],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "baz1"
// CHECK-NEXT:           },
// CHECK-NEXT:           "expression": true,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

x < (foo.bar.baz ?? 0) - 1 ? foo.bar : null;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ConditionalExpression",
// CHECK-NEXT:         "test": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "BinaryExpression",
// CHECK-NEXT:             "left": {
// CHECK-NEXT:               "type": "LogicalExpression",
// CHECK-NEXT:               "left": {
// CHECK-NEXT:                 "type": "MemberExpression",
// CHECK-NEXT:                 "object": {
// CHECK-NEXT:                   "type": "MemberExpression",
// CHECK-NEXT:                   "object": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "foo"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "property": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "bar"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "computed": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "property": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "baz"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "computed": false
// CHECK-NEXT:               },
// CHECK-NEXT:               "right": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 0,
// CHECK-NEXT:                 "raw": "0"
// CHECK-NEXT:               },
// CHECK-NEXT:               "operator": "??"
// CHECK-NEXT:             },
// CHECK-NEXT:             "right": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 1,
// CHECK-NEXT:               "raw": "1"
// CHECK-NEXT:             },
// CHECK-NEXT:             "operator": "-"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "<"
// CHECK-NEXT:         },
// CHECK-NEXT:         "alternate": {
// CHECK-NEXT:           "type": "NullLiteral"
// CHECK-NEXT:         },
// CHECK-NEXT:         "consequent": {
// CHECK-NEXT:           "type": "MemberExpression",
// CHECK-NEXT:           "object": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "property": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "bar"
// CHECK-NEXT:           },
// CHECK-NEXT:           "computed": false
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
