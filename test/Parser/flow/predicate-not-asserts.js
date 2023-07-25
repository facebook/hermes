/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

type asserts = string;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "asserts"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "StringTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

function foo(): asserts | number {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "FunctionDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "foo"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "returnType": {
// CHECK-NEXT:         "type": "TypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "UnionTypeAnnotation",
// CHECK-NEXT:           "types": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "GenericTypeAnnotation",
// CHECK-NEXT:               "id": {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "asserts"
// CHECK-NEXT:               },
// CHECK-NEXT:               "typeParameters": null
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "NumberTypeAnnotation"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "generator": false,
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
