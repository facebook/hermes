/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

"use strict";
({yield: x} = {yield: 1});

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "use strict"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": "use strict"
// CHECK-NEXT:     },
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "AssignmentExpression",
// CHECK-NEXT:         "operator": "=",
// CHECK-NEXT:         "left": {
// CHECK-NEXT:           "type": "ObjectPattern",
// CHECK-NEXT:           "properties": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Property",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "yield"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "x"
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "init",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "method": false,
// CHECK-NEXT:               "shorthand": false
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "right": {
// CHECK-NEXT:           "type": "ObjectExpression",
// CHECK-NEXT:           "properties": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Property",
// CHECK-NEXT:               "key": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "yield"
// CHECK-NEXT:               },
// CHECK-NEXT:               "value": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 1,
// CHECK-NEXT:                 "raw": "1"
// CHECK-NEXT:               },
// CHECK-NEXT:               "kind": "init",
// CHECK-NEXT:               "computed": false,
// CHECK-NEXT:               "method": false,
// CHECK-NEXT:               "shorthand": false
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
