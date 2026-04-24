/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// `in` is reserved at the lexer level (rw_in) for the JS `in` operator and
// for-in loops, but Flow allows it as a type-parameter name (it's
// reclassified as an identifier in TYPE lex mode). Hermes accepts the same
// — narrowly, just `in`. Other reswords (e.g. `return`, `class`, `void`,
// `const`) are not accepted as type-param names; they have parser-level
// meaning elsewhere in type context that would break with broader
// reclassification.

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

// `in` as the sole type-parameter name.
type T1<in> = void;
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
// CHECK-NEXT:             "name": "in",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "VoidTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// `in` mixed with regular identifier names in the same declaration.
type T2<X, in, Y> = void;
// CHECK-NEXT:     {
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
// CHECK-NEXT:             "name": "in",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           },
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "Y",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "VoidTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     },

// `in` as the name with a colon bound.
type T3<in: number> = void;
// CHECK-NEXT:     {
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
// CHECK-NEXT:             "bound": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "right": {
// CHECK-NEXT:         "type": "VoidTypeAnnotation"
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK:        ]
// CHECK-NEXT: }
