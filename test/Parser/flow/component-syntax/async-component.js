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

async component Foo1() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo1"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

export default async component Foo2() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExportDefaultDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "ComponentDeclaration",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "Foo2"
// CHECK-NEXT:         },
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         },
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       }
// CHECK-NEXT:     },

export async component Foo3() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExportNamedDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "ComponentDeclaration",
// CHECK-NEXT:         "id": {
// CHECK-NEXT:           "type": "Identifier",
// CHECK-NEXT:           "name": "Foo3"
// CHECK-NEXT:         },
// CHECK-NEXT:         "params": [],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         },
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "specifiers": [],
// CHECK-NEXT:       "source": null,
// CHECK-NEXT:       "exportKind": "value"
// CHECK-NEXT:     },

async component Foo4(foo: string) {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo4"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ComponentParameter",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "local": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "StringTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "shorthand": true
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

async component Foo5() renders SomeComponent {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo5"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
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
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

async component Foo6() renders? SomeComponent {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo6"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
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
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

async component Foo7() renders* SomeComponent {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo7"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
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
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

async component Foo8<T>() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo8"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
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
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

async component Foo9<T>(foo: T) renders SomeComponent {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo9"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ComponentParameter",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "local": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "GenericTypeAnnotation",
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "T"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "shorthand": true
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
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
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": true
// CHECK-NEXT:     },

// Ensure `async component => {}` is parsed as an async arrow function,
// not as an async component declaration.
async component => {};
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "ArrowFunctionExpression",
// CHECK-NEXT:         "params": [
// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "component"
// CHECK-NEXT:           }
// CHECK-NEXT:         ],
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         },
// CHECK-NEXT:         "expression": false,
// CHECK-NEXT:         "async": true
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },

// Ensure `async\ncomponent Foo() {}` is not parsed as an async component
// declaration because of the line terminator after `async`.
async
component Foo10() {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExpressionStatement",
// CHECK-NEXT:       "expression": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "async"
// CHECK-NEXT:       },
// CHECK-NEXT:       "directive": null
// CHECK-NEXT:     },
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo10"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "async": false
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
