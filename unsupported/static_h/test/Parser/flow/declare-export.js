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

declare export default function foo(): number;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareExportDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "DeclareFunction",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "FunctionTypeAnnotation",
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "this": null,
// CHECK-NEXT:                 "returnType": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "rest": null,
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "predicate": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "default": true
// CHECK-NEXT:       },

declare export default class Foo {}
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareExportDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "DeclareClass",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "extends": [],
// CHECK-NEXT:           "implements": [],
// CHECK-NEXT:           "mixins": [],
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
// CHECK-NEXT:         "default": true
// CHECK-NEXT:       },

declare export default Foo;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareExportDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "default": true
// CHECK-NEXT:       },

declare export function foo(): number;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareExportDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "DeclareFunction",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "FunctionTypeAnnotation",
// CHECK-NEXT:                 "params": [],
// CHECK-NEXT:                 "this": null,
// CHECK-NEXT:                 "returnType": {
// CHECK-NEXT:                   "type": "NumberTypeAnnotation"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "rest": null,
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "predicate": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "default": false
// CHECK-NEXT:       },

declare export var x: number;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareExportDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "DeclareVariable",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "x",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "default": false
// CHECK-NEXT:       },

declare export {x as y} from 'foo';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareExportDeclaration",
// CHECK-NEXT:         "declaration": null,
// CHECK-NEXT:         "specifiers": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ExportSpecifier",
// CHECK-NEXT:             "exported": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "y"
// CHECK-NEXT:             },
// CHECK-NEXT:             "local": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "x"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         },
// CHECK-NEXT:         "default": false
// CHECK-NEXT:       },

declare export opaque type T;
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareExportDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "DeclareOpaqueType",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "T"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null,
// CHECK-NEXT:           "impltype": null,
// CHECK-NEXT:           "supertype": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "specifiers": [],
// CHECK-NEXT:         "source": null,
// CHECK-NEXT:         "default": false
// CHECK-NEXT:       },

declare export * from 'foo';
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "DeclareExportAllDeclaration",
// CHECK-NEXT:         "source": {
// CHECK-NEXT:           "type": "StringLiteral",
// CHECK-NEXT:           "value": "foo"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
