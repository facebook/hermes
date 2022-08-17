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

declare class A mixins B {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "mixins": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "InterfaceExtends",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "B"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare class A mixins B implements C {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareClass",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "extends": [],
// CHECK-NEXT:       "implements": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ClassImplements",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "C"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "mixins": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "InterfaceExtends",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "B"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "ObjectTypeAnnotation",
// CHECK-NEXT:         "properties": [],
// CHECK-NEXT:         "indexers": [],
// CHECK-NEXT:         "callProperties": [],
// CHECK-NEXT:         "internalSlots": [],
// CHECK-NEXT:         "inexact": false,
// CHECK-NEXT:         "exact": false
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
