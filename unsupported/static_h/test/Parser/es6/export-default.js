/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -commonjs -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

export default 2 + 2;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportDefaultDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "BinaryExpression",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 2,
// CHECK-NEXT:             "raw": "2"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 2,
// CHECK-NEXT:             "raw": "2"
// CHECK-NEXT:           },
// CHECK-NEXT:           "operator": "+"
// CHECK-NEXT:         }
// CHECK-NEXT:       }

// CHECK-NEXT:     ]
// CHECK-NEXT:   },
