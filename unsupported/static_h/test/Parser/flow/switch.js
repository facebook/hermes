/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

function foo() {
  switch (x) {
    case 1:
      return 1;
    case (x):
      return 1;
    case (): number => {3}:
      return 1;
    case (y: number):
      return 1;
  }
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "SwitchStatement",
// CHECK-NEXT:             "discriminant": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "cases": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "SwitchCase",
// CHECK-NEXT:                 "test": {
// CHECK-NEXT:                   "type": "NumericLiteral",
// CHECK-NEXT:                   "value": 1,
// CHECK-NEXT:                   "raw": "1"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "consequent": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ReturnStatement",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 1,
// CHECK-NEXT:                       "raw": "1"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "SwitchCase",
// CHECK-NEXT:                 "test": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "x"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "consequent": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ReturnStatement",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 1,
// CHECK-NEXT:                       "raw": "1"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "SwitchCase",
// CHECK-NEXT:                 "test": {
// CHECK-NEXT:                   "type": "ArrowFunctionExpression",
// CHECK-NEXT:                   "id": null,
// CHECK-NEXT:                   "params": [],
// CHECK-NEXT:                   "body": {
// CHECK-NEXT:                     "type": "BlockStatement",
// CHECK-NEXT:                     "body": [
// CHECK-NEXT:                       {
// CHECK-NEXT:                         "type": "ExpressionStatement",
// CHECK-NEXT:                         "expression": {
// CHECK-NEXT:                           "type": "NumericLiteral",
// CHECK-NEXT:                           "value": 3,
// CHECK-NEXT:                           "raw": "3"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "directive": null
// CHECK-NEXT:                       }
// CHECK-NEXT:                     ]
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "returnType": {
// CHECK-NEXT:                     "type": "TypeAnnotation",
// CHECK-NEXT:                     "typeAnnotation": {
// CHECK-NEXT:                       "type": "NumberTypeAnnotation"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "expression": false,
// CHECK-NEXT:                   "async": false
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "consequent": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ReturnStatement",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 1,
// CHECK-NEXT:                       "raw": "1"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "SwitchCase",
// CHECK-NEXT:                 "test": {
// CHECK-NEXT:                   "type": "TypeCastExpression",
// CHECK-NEXT:                   "expression": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "y"
// CHECK-NEXT:                   },
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TypeAnnotation",
// CHECK-NEXT:                     "typeAnnotation": {
// CHECK-NEXT:                       "type": "NumberTypeAnnotation"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "consequent": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ReturnStatement",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 1,
// CHECK-NEXT:                       "raw": "1"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     }
