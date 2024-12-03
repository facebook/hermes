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

class C {
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [

  static {}
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "StaticBlock",
// CHECK-NEXT:             "body": []
// CHECK-NEXT:           },

  static {
    let x = 1;
  }
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "StaticBlock",
// CHECK-NEXT:             "body": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "VariableDeclaration",
// CHECK-NEXT:                 "kind": "let",
// CHECK-NEXT:                 "declarations": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "VariableDeclarator",
// CHECK-NEXT:                     "init": {
// CHECK-NEXT:                       "type": "NumericLiteral",
// CHECK-NEXT:                       "value": 1,
// CHECK-NEXT:                       "raw": "1"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "id": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "x"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }

}
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
