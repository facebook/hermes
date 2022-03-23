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

type X = A;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "X"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSTypeReference",
// CHECK-NEXT:         "typeName": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "A"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type X = A.B;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "X"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSTypeReference",
// CHECK-NEXT:         "typeName": {
// CHECK-NEXT:           "type": "TSQualifiedName",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "A"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "B"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type X = A.B.C;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "X"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSTypeReference",
// CHECK-NEXT:         "typeName": {
// CHECK-NEXT:           "type": "TSQualifiedName",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "TSQualifiedName",
// CHECK-NEXT:             "left": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "A"
// CHECK-NEXT:             },
// CHECK-NEXT:             "right": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "B"
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "C"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type X = A<B>;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "X"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSTypeReference",
// CHECK-NEXT:         "typeName": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "A"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TSTypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TSTypeReference",
// CHECK-NEXT:               "typeName": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "B"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type X = A<B<C>>;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "X"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSTypeReference",
// CHECK-NEXT:         "typeName": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "A"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TSTypeParameterInstantiation",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TSTypeReference",
// CHECK-NEXT:               "typeName": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "B"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": {
// CHECK-NEXT:                 "type": "TSTypeParameterInstantiation",
// CHECK-NEXT:                 "params": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "TSTypeReference",
// CHECK-NEXT:                     "typeName": {
// CHECK-NEXT:                       "type": "Identifier",
// CHECK-NEXT:                       "name": "C"
// CHECK-NEXT:                     },
// CHECK-NEXT:                     "typeParameters": null
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
