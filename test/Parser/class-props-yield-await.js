/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [
// CHECK-NEXT:     {

async function* foo() {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [

  class C {
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassDeclaration",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "C"
// CHECK-NEXT:             },
// CHECK-NEXT:             "superClass": null,
// CHECK-NEXT:             "body": {
// CHECK-NEXT:               "type": "ClassBody",
// CHECK-NEXT:               "body": [

    [yield 1] = 1;
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "ClassProperty",
// CHECK-NEXT:                   "key": {
// CHECK-NEXT:                     "type": "YieldExpression",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 1,
// CHECK-NEXT:                       "raw": "1"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "delegate": false
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "value": {
// CHECK-NEXT:                     "type": "NumericLiteral",
// CHECK-NEXT:                     "value": 1,
// CHECK-NEXT:                     "raw": "1"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "computed": true,
// CHECK-NEXT:                   "static": false,
// CHECK-NEXT:                   "declare": false
// CHECK-NEXT:                 },

    [await 1] = 1;
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "ClassProperty",
// CHECK-NEXT:                   "key": {
// CHECK-NEXT:                     "type": "AwaitExpression",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 1,
// CHECK-NEXT:                       "raw": "1"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "value": {
// CHECK-NEXT:                     "type": "NumericLiteral",
// CHECK-NEXT:                     "value": 1,
// CHECK-NEXT:                     "raw": "1"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "computed": true,
// CHECK-NEXT:                   "static": false,
// CHECK-NEXT:                   "declare": false
// CHECK-NEXT:                 }

  }
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]

}
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": true,
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
