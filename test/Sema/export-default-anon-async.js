/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -commonjs -dump-transformed-ast -pretty-json %s | %FileCheck --match-full-lines %s

// Regression test: the rewrite of an anonymous `export default function` to a
// FunctionExpression used to pass a hard-coded `async = false`, so an
// anonymous `export default async function` was compiled as a sync function
// and its own `await` was reported as an error.

export default async function () {
  await 0;
}

// CHECK-LABEL:         "type": "ExportDefaultDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "FunctionExpression",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "params": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "BlockStatement",
// CHECK-NEXT:             "body": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "ExpressionStatement",
// CHECK-NEXT:                 "expression": {
// CHECK-NEXT:                   "type": "AwaitExpression",
// CHECK-NEXT:                   "argument": {
// CHECK-NEXT:                     "type": "NumericLiteral",
// CHECK-NEXT:                     "value": 0,
// CHECK-NEXT:                     "raw": "0"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "directive": null
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "generator": false,
// CHECK-NEXT:           "async": true
// CHECK-NEXT:         }
