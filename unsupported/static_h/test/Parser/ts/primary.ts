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

type A = number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSNumberKeyword"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = string;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSStringKeyword"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = 1;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSLiteralType",
// CHECK-NEXT:         "literal": {
// CHECK-NEXT:           "type": "NumericLiteral",
// CHECK-NEXT:           "value": 1,
// CHECK-NEXT:           "raw": "1"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = 'asdf';
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSLiteralType",
// CHECK-NEXT:         "literal": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "asdf"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = this;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSThisType"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (number);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSNumberKeyword"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = ((((number))));
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSNumberKeyword"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (number) => string;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSFunctionType",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "number"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TSStringKeyword"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = (number) => string;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSFunctionType",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "number"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "returnType": {
// CHECK-NEXT:           "type": "TSStringKeyword"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = typeof B;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSTypeQuery",
// CHECK-NEXT:         "exprName": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "B"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = typeof B.C;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TSTypeAliasDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TSTypeQuery",
// CHECK-NEXT:         "exprName": {
// CHECK-NEXT:           "type": "TSQualifiedName",
// CHECK-NEXT:           "left": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "B"
// CHECK-NEXT:           },
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "C"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
