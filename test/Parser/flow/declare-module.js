/**
 * Copyright (c) Facebook, Inc. and its affiliates.
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
