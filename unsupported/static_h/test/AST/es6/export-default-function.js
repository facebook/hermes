/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-transformed-ast -commonjs -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

'use strict';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExpressionStatement",
// CHECK-NEXT:         "expression": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "use strict"
// CHECK-NEXT:         },
// CHECK-NEXT:         "directive": "use strict"
// CHECK-NEXT:       },

export default function() {}
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportDefaultDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "FunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "BlockStatement",
// CHECK-NEXT:             "body": []
// CHECK-NEXT:           },
// CHECK-NEXT:           "generator": false,
// CHECK-NEXT:           "async": false
// CHECK-NEXT:         }
// CHECK-NEXT:       }

// CHECK-NEXT:     ]
// CHECK-NEXT:   },
// CHECK-NEXT:   "generator": false,
// CHECK-NEXT:   "async": false
// CHECK-NEXT: }
