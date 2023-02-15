/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines

async function fn() {
// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "fn"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [

  for (async of => 1; ; );
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ForStatement",
// CHECK-NEXT:             "init": {
// CHECK-NEXT:               "type": "ArrowFunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "of"
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "NumericLiteral",
// CHECK-NEXT:                 "value": 1,
// CHECK-NEXT:                 "raw": "1"
// CHECK-NEXT:               },
// CHECK-NEXT:               "expression": true,
// CHECK-NEXT:               "async": true
// CHECK-NEXT:             },
// CHECK-NEXT:             "test": null,
// CHECK-NEXT:             "update": null,
// CHECK-NEXT:             "body": {
// CHECK-NEXT:               "type": "EmptyStatement"
// CHECK-NEXT:             }
// CHECK-NEXT:           },

  for await (async of 1);
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ForOfStatement",
// CHECK-NEXT:             "left": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "async"
// CHECK-NEXT:             },
// CHECK-NEXT:             "right": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 1,
// CHECK-NEXT:               "raw": "1"
// CHECK-NEXT:             },
// CHECK-NEXT:             "body": {
// CHECK-NEXT:               "type": "EmptyStatement"
// CHECK-NEXT:             },
// CHECK-NEXT:             "await": true
// CHECK-NEXT:           }

}
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
