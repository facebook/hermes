/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

switch (x) {
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "SwitchStatement",
// CHECK-NEXT:       "discriminant": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "x"
// CHECK-NEXT:       },
// CHECK-NEXT:       "cases": [

  case 0:
    let a;
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "SwitchCase",
// CHECK-NEXT:           "test": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 0,
// CHECK-NEXT:             "raw": "0"
// CHECK-NEXT:           },
// CHECK-NEXT:           "consequent": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "VariableDeclaration",
// CHECK-NEXT:               "kind": "let",
// CHECK-NEXT:               "declarations": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "VariableDeclarator",
// CHECK-NEXT:                   "init": null,
// CHECK-NEXT:                   "id": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "a"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },

  case 1: {
    const b = 3;
  }
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "SwitchCase",
// CHECK-NEXT:           "test": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 1,
// CHECK-NEXT:             "raw": "1"
// CHECK-NEXT:           },
// CHECK-NEXT:           "consequent": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "BlockStatement",
// CHECK-NEXT:               "body": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "VariableDeclaration",
// CHECK-NEXT:                   "kind": "const",
// CHECK-NEXT:                   "declarations": [
// CHECK-NEXT:                     {
// CHECK-NEXT:                       "type": "VariableDeclarator",
// CHECK-NEXT:                       "init": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 3,
// CHECK-NEXT:                         "raw": "3"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "id": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "b"
// CHECK-NEXT:                       }
// CHECK-NEXT:                     }
// CHECK-NEXT:                   ]
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },

  case 2:
    let c;
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "SwitchCase",
// CHECK-NEXT:           "test": {
// CHECK-NEXT:             "type": "NumericLiteral",
// CHECK-NEXT:             "value": 2,
// CHECK-NEXT:             "raw": "2"
// CHECK-NEXT:           },
// CHECK-NEXT:           "consequent": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "VariableDeclaration",
// CHECK-NEXT:               "kind": "let",
// CHECK-NEXT:               "declarations": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "VariableDeclarator",
// CHECK-NEXT:                   "init": null,
// CHECK-NEXT:                   "id": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "c"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },

  default:
    let d;
}
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "SwitchCase",
// CHECK-NEXT:           "test": null,
// CHECK-NEXT:           "consequent": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "VariableDeclaration",
// CHECK-NEXT:               "kind": "let",
// CHECK-NEXT:               "declarations": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "VariableDeclarator",
// CHECK-NEXT:                   "init": null,
// CHECK-NEXT:                   "id": {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "d"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               ]
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }

// CHECK-NEXT:       ]
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
