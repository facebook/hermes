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

declare module.exports: number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareModuleExports",
// CHECK-NEXT:       "typeAnnotation": {
// CHECK-NEXT:         "type": "TypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare module Foo {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareModule",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "kind": "CommonJS"
// CHECK-NEXT:     },

declare module "Foo" {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareModule",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "StringLiteral",
// CHECK-NEXT:         "value": "Foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "kind": "CommonJS"
// CHECK-NEXT:     },

declare module Mod {
  declare export type T = string;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareModule",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Mod"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "DeclareExportDeclaration",
// CHECK-NEXT:             "declaration": {
// CHECK-NEXT:               "type": "TypeAlias",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "T"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null,
// CHECK-NEXT:               "right": {
// CHECK-NEXT:                 "type": "StringTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "specifiers": [],
// CHECK-NEXT:             "source": null,
// CHECK-NEXT:             "default": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "kind": "CommonJS"
// CHECK-NEXT:     },

declare module Mod {
  declare export * from 'foo';
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareModule",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Mod"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "DeclareExportAllDeclaration",
// CHECK-NEXT:             "source": {
// CHECK-NEXT:               "type": "StringLiteral",
// CHECK-NEXT:               "value": "foo"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "kind": "ES"
// CHECK-NEXT:     },

declare module Mod {
  declare export interface bar {}
  declare export var baz: number;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareModule",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Mod"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "DeclareExportDeclaration",
// CHECK-NEXT:             "declaration": {
// CHECK-NEXT:               "type": "InterfaceDeclaration",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "bar"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null,
// CHECK-NEXT:               "extends": [],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "ObjectTypeAnnotation",
// CHECK-NEXT:                 "properties": [],
// CHECK-NEXT:                 "indexers": [],
// CHECK-NEXT:                 "callProperties": [],
// CHECK-NEXT:                 "internalSlots": [],
// CHECK-NEXT:                 "inexact": false,
// CHECK-NEXT:                 "exact": false
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "specifiers": [],
// CHECK-NEXT:             "source": null,
// CHECK-NEXT:             "default": false
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "DeclareExportDeclaration",
// CHECK-NEXT:             "declaration": {
// CHECK-NEXT:               "type": "DeclareVariable",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "baz",
// CHECK-NEXT:                 "typeAnnotation": {
// CHECK-NEXT:                   "type": "TypeAnnotation",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "NumberTypeAnnotation"
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "specifiers": [],
// CHECK-NEXT:             "source": null,
// CHECK-NEXT:             "default": false
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "kind": "ES"
// CHECK-NEXT:     },

declare module WithImport {
  import type T from "Foo";
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareModule",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "WithImport"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "ImportDeclaration",
// CHECK-NEXT:             "specifiers": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "ImportDefaultSpecifier",
// CHECK-NEXT:                 "local": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "T"
// CHECK-NEXT:                 }
// CHECK-NEXT:               }
// CHECK-NEXT:             ],
// CHECK-NEXT:             "source": {
// CHECK-NEXT:               "type": "StringLiteral",
// CHECK-NEXT:               "value": "Foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "assertions": [],
// CHECK-NEXT:             "importKind": "type"
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "kind": "CommonJS"
// CHECK-NEXT:     },

declare module Bar {
  declare module.exports: number;
  declare type T = number;
}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareModule",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Bar"
// CHECK-NEXT:       },
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "DeclareModuleExports",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "DeclareTypeAlias",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "T"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null,
// CHECK-NEXT:             "right": {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "kind": "CommonJS"
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
