/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

export type A = number;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "TypeAlias",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "A"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "right": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "type"
// CHECK-NEXT:       },

export opaque type B = number;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "OpaqueType",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "B"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "impltype": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           },
// CHECK-NEXT:           "supertype": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "type"
// CHECK-NEXT:       },

export interface T<U> { }
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "InterfaceDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "T"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": {
// CHECK-NEXT:             "type": "TypeParameterDeclaration",
// CHECK-NEXT:             "params": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "TypeParameter",
// CHECK-NEXT:                 "name": "U",
// CHECK-NEXT:                 "bound": null,
// CHECK-NEXT:                 "variance": null,
// CHECK-NEXT:                 "default": null
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "extends": [],
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "ObjectTypeAnnotation",
// CHECK-NEXT:             "properties": [],
// CHECK-NEXT:             "indexers": [],
// CHECK-NEXT:             "callProperties": [],
// CHECK-NEXT:             "internalSlots": [],
// CHECK-NEXT:             "inexact": false,
// CHECK-NEXT:             "exact": false
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "type"
// CHECK-NEXT:       },

export type * from "foo";
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportAllDeclaration",
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "exportKind": "type"
// CHECK-NEXT:       },

export type {type as type};
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": null,
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExportSpecifier",
// CHECK-NEXT:             "exported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "type"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "type"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "type"
// CHECK-NEXT:       },

export {type as type};
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": null,
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExportSpecifier",
// CHECK-NEXT:             "exported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "type"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "type"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export enum X {}
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportNamedDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "EnumDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "X"
// CHECK-NEXT:           },
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "EnumStringBody",
// CHECK-NEXT:             "members": [],
// CHECK-NEXT:             "explicitType": false,
// CHECK-NEXT:             "hasUnknownMembers": false
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "exportKind": "value"
// CHECK-NEXT:       },

export default enum X {}
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportDefaultDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "EnumDeclaration",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "X"
// CHECK-NEXT:           },
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "EnumStringBody",
// CHECK-NEXT:             "members": [],
// CHECK-NEXT:             "explicitType": false,
// CHECK-NEXT:             "hasUnknownMembers": false
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       }
