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

declare component Foo1();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareComponent",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "rest": null,
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "rendersType": null
// CHECK-NEXT:     },

declare export default component Foo2();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "DeclareComponent",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "Foo2"
// CHECK-NEXT:         },
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "rendersType": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "default": true
// CHECK-NEXT:     },

declare export component Foo3();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareExportDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "DeclareComponent",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "Foo3"
// CHECK-NEXT:         },
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "rest": null,
// CHECK-NEXT:         "typeParameters": null,
// CHECK-NEXT:         "rendersType": null
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "default": false
// CHECK-NEXT:     },

declare component Foo1() renders SomeComponent;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareComponent",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "rest": null,
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "rendersType": {
// CHECK-NEXT:         "type": "TypeOperator",
// CHECK-NEXT:         "operator": "renders",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "SomeComponent"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare component Foo1<T>();
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareComponent",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "rest": null,
// CHECK-NEXT:       "typeParameters": {
// CHECK-NEXT:         "type": "TypeParameterDeclaration",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "TypeParameter",
// CHECK-NEXT:             "name": "T",
// CHECK-NEXT:             "bound": null,
// CHECK-NEXT:             "variance": null,
// CHECK-NEXT:             "default": null
// CHECK-NEXT:           }
// CHECK-NEXT:         ]
// CHECK-NEXT:       },
// CHECK-NEXT:       "rendersType": null
// CHECK-NEXT:     },

declare component Foo1() renders? SomeComponent;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareComponent",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "rest": null,
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "rendersType": {
// CHECK-NEXT:         "type": "TypeOperator",
// CHECK-NEXT:         "operator": "renders?",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "SomeComponent"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     },

declare component Foo1() renders* SomeComponent;
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "DeclareComponent",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "rest": null,
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "rendersType": {
// CHECK-NEXT:         "type": "TypeOperator",
// CHECK-NEXT:         "operator": "renders*",
// CHECK-NEXT:         "typeAnnotation": {
// CHECK-NEXT:           "type": "GenericTypeAnnotation",
// CHECK-NEXT:           "id": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "SomeComponent"
// CHECK-NEXT:           },
// CHECK-NEXT:           "typeParameters": null
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
