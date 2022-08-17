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

interface A<T> extends C<U>, D<W> {
  a: number,
  b: (number) => string,
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSInterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "TSInterfaceBody",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSPropertySignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSNumberKeyword"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "initializer": null,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "readonly": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "export": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TSPropertySignature",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TSTypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "TSFunctionType",
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "Identifier",
// CHECK-NEXT:                     "name": "number"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ],
// CHECK-NEXT:                 "returnType": {
// CHECK-NEXT:                   "type": "TSStringKeyword"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "initializer": null,
// CHECK-NEXT:             "optional": false,
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "readonly": false,
// CHECK-NEXT:             "static": false,
// CHECK-NEXT:             "export": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "extends": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "TSInterfaceHeritage",
// CHECK-NEXT:           "expression": {
// CHECK-NEXT:             "type": "TSTypeReference",
// CHECK-NEXT:             "typeName": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "C"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": {
// CHECK-NEXT:             "type": "TSTypeParameterInstantiation",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "TSTypeReference",
// CHECK-NEXT:                 "typeName": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "U"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "TSInterfaceHeritage",
// CHECK-NEXT:           "expression": {
// CHECK-NEXT:             "type": "TSTypeReference",
// CHECK-NEXT:             "typeName": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "D"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": {
// CHECK-NEXT:             "type": "TSTypeParameterInstantiation",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "TSTypeReference",
// CHECK-NEXT:                 "typeName": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "W"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
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
// CHECK-NEXT:       }
// CHECK-NEXT:     },

interface A { }
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSInterfaceDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "TSInterfaceBody",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "typeParameters": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
