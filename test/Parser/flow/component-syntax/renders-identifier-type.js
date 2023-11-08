/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-component-syntax -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

type T = (renders: string) => void;
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "TypeAlias",
// CHECK-NEXT:        "id": {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "T"
// CHECK-NEXT:        },
// CHECK-NEXT:        "typeParameters": null,
// CHECK-NEXT:        "right": {
// CHECK-NEXT:          "type": "FunctionTypeAnnotation",
// CHECK-NEXT:          "params": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "FunctionTypeParam",
// CHECK-NEXT:              "name": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "renders"
// CHECK-NEXT:              },
// CHECK-NEXT:              "typeAnnotation": {
// CHECK-NEXT:                "type": "StringTypeAnnotation"
// CHECK-NEXT:              },
// CHECK-NEXT:              "optional": false
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "this": null,
// CHECK-NEXT:          "returnType": {
// CHECK-NEXT:            "type": "VoidTypeAnnotation"
// CHECK-NEXT:          },
// CHECK-NEXT:          "rest": null,
// CHECK-NEXT:          "typeParameters": null
// CHECK-NEXT:        }
// CHECK-NEXT:      },

type T = {[renders: string]: void};
// CHECK-NEXT:      {
// CHECK-NEXT:        "type": "TypeAlias",
// CHECK-NEXT:        "id": {
// CHECK-NEXT:          "type": "Identifier",
// CHECK-NEXT:          "name": "T"
// CHECK-NEXT:        },
// CHECK-NEXT:        "typeParameters": null,
// CHECK-NEXT:        "right": {
// CHECK-NEXT:          "type": "ObjectTypeAnnotation",
// CHECK-NEXT:          "properties": [],
// CHECK-NEXT:          "indexers": [
// CHECK-NEXT:            {
// CHECK-NEXT:              "type": "ObjectTypeIndexer",
// CHECK-NEXT:              "id": {
// CHECK-NEXT:                "type": "Identifier",
// CHECK-NEXT:                "name": "renders"
// CHECK-NEXT:              },
// CHECK-NEXT:              "key": {
// CHECK-NEXT:                "type": "StringTypeAnnotation"
// CHECK-NEXT:              },
// CHECK-NEXT:              "value": {
// CHECK-NEXT:                "type": "VoidTypeAnnotation"
// CHECK-NEXT:              },
// CHECK-NEXT:              "static": false,
// CHECK-NEXT:              "variance": null
// CHECK-NEXT:            }
// CHECK-NEXT:          ],
// CHECK-NEXT:          "callProperties": [],
// CHECK-NEXT:          "internalSlots": [],
// CHECK-NEXT:          "inexact": false,
// CHECK-NEXT:          "exact": false
// CHECK-NEXT:        }
// CHECK-NEXT:      }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
