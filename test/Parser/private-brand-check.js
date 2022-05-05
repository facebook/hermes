/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

class C {
  foo() {
// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [
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
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [

    #foo in this;
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ExpressionStatement",
// CHECK-NEXT:                     "expression": {
// CHECK-NEXT:                       "type": "BinaryExpression",
// CHECK-NEXT:                       "left": {
// CHECK-NEXT:                         "type": "PrivateName",
// CHECK-NEXT:                         "id": {
// CHECK-NEXT:                           "type": "Identifier",
// CHECK-NEXT:                           "name": "foo"
// CHECK-NEXT:                         }
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "right": {
// CHECK-NEXT:                         "type": "ThisExpression"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "operator": "in"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   },

    1 == #foo in this;
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ExpressionStatement",
// CHECK-NEXT:                     "expression": {
// CHECK-NEXT:                       "type": "BinaryExpression",
// CHECK-NEXT:                       "left": {
// CHECK-NEXT:                         "type": "NumericLiteral",
// CHECK-NEXT:                         "value": 1,
// CHECK-NEXT:                         "raw": "1"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "right": {
// CHECK-NEXT:                         "type": "BinaryExpression",
// CHECK-NEXT:                         "left": {
// CHECK-NEXT:                           "type": "PrivateName",
// CHECK-NEXT:                           "id": {
// CHECK-NEXT:                             "type": "Identifier",
// CHECK-NEXT:                             "name": "foo"
// CHECK-NEXT:                           }
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "right": {
// CHECK-NEXT:                           "type": "ThisExpression"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "operator": "in"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "operator": "=="
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "directive": null
// CHECK-NEXT:                   }

  }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },

}
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
