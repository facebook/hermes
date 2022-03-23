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

type A = ?number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NullableTypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "NumberTypeAnnotation"
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type A = ??number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "A"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NullableTypeAnnotation",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "NullableTypeAnnotation",
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "NumberTypeAnnotation"
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
