/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -parse-ts -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

class C<T> extends D<U> {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "C"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TSTypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSTypeParameter",
// CHECK-NEXT:             "name": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "T"
// CHECK-NEXT:             },
// CHECK-NEXT:             "constraint": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "D"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superTypeParameters": {
// CHECK-NEXT:         "type": "TSTypeParameterInstantiation",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSTypeReference",
// CHECK-NEXT:             "typeName": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "U"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       }
// CHECK-NEXT:     },

(class C<T> extends D<U> {});
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ClassExpression",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "C"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TSTypeParameterDeclaration",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TSTypeParameter",
// CHECK-NEXT:               "name": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "T"
// CHECK-NEXT:               },
// CHECK-NEXT:               "constraint": null,
// CHECK-NEXT:               "default": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "superClass": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "D"
// CHECK-NEXT:         },
// CHECK-NEXT:         "superTypeParameters": {
// CHECK-NEXT:           "type": "TSTypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TSTypeReference",
// CHECK-NEXT:               "typeName": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "U"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ClassBody",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

class Foo {
  private = 3;
  private x = 3;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ClassDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "superClass": null,
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ClassBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "private"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 3,
// CHECK-NEXT:               "raw": "3"
// CHECK-NEXT:             },
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ClassPrivateProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 3,
// CHECK-NEXT:               "raw": "3"
// CHECK-NEXT:             },
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "declare": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
