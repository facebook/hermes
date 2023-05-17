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

declare component Foo1(...Foo);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareComponent",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "rest": {
// CHECK-NEXT:         "type": "ComponentTypeParameter",
// CHECK-NEXT:         "name": null,
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "rendersType": null
// CHECK-NEXT:     },

declare component Foo2(...rest: Foo);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareComponent",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo2"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "rest": {
// CHECK-NEXT:         "type": "ComponentTypeParameter",
// CHECK-NEXT:         "name": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "rest"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "rendersType": null
// CHECK-NEXT:     },

declare component Foo3(...rest?: Foo);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareComponent",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo3"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "rest": {
// CHECK-NEXT:         "type": "ComponentTypeParameter",
// CHECK-NEXT:         "name": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "rest"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "optional": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "rendersType": null
// CHECK-NEXT:     },

declare component Foo4(foo: Foo, ...rest: Rest);
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareComponent",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo4"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ComponentTypeParameter",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeAnnotation": {
// CHECK-NEXT:             "type": "GenericTypeAnnotation",
// CHECK-NEXT:             "id": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "Foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeParameters": null
// CHECK-NEXT:           },
// CHECK-NEXT:           "optional": false
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "rest": {
// CHECK-NEXT:         "type": "ComponentTypeParameter",
// CHECK-NEXT:         "name": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "rest"
// CHECK-NEXT:         },
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "Rest"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         },
// CHECK-NEXT:         "optional": false
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "rendersType": null
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
