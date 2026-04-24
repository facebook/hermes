/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// `in T`: long-form spelling of `-T` (contravariance) on a type
// parameter, matching TS. Disambiguated from `<in>` (where `in` is itself
// the name): `in` is consumed speculatively, and if the next token is a
// valid name token (identifier or another `in`), `in` was variance and
// the next token is the name. Otherwise `in` itself is the name.

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

// `in T`: variance "in" on a type parameter named "T".
type T1<in T> = (x: T) => void;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "T",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "in"
// CHECK-NEXT:             },
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK:        }

// `in` mixed with other type parameters: variance applies only to `Y`.
type T2<X, in Y, Z> = [X, Y, Z];
// CHECK:        {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T2"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "X",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "Y",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "in"
// CHECK-NEXT:             },
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "Z",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK:        }

// Edge case: `<in in>` — variance "in" on a type parameter literally named
// "in". Both lookahead-eligible token kinds (rw_in) flow through correctly.
type T3<in in> = void;
// CHECK:        {
// CHECK-NEXT:       "type": "TypeAlias",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "T3"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "in",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": {
// CHECK-NEXT:               "type": "Variance",
// CHECK-NEXT:               "kind": "in"
// CHECK-NEXT:             },
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "VoidTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK:      ]
// CHECK-NEXT: }
