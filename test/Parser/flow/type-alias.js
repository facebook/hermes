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

type T = number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NumberTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

type T = function;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "function"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare type T = number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareTypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "NumberTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

opaque type T = number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "OpaqueType",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "impltype": {
// CHECK-NEXT:         "type": "NumberTypeAnnotation"
// CHECK-NEXT:       },
// CHECK-NEXT:       "supertype": null
// CHECK-NEXT:     },

opaque type T: U = number;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "OpaqueType",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "impltype": {
// CHECK-NEXT:         "type": "NumberTypeAnnotation"
// CHECK-NEXT:       },
// CHECK-NEXT:       "supertype": {
// CHECK-NEXT:         "type": "GenericTypeAnnotation",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "U"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeParameters": null
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
