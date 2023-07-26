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

component Foo1(foo as bar) {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo1"
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
// CHECK-NEXT:             "name": "bar"
// CHECK-NEXT:           },
// CHECK-NEXT:           "shorthand": false
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       }
// CHECK-NEXT:     },

component Foo2(foo as bar: Foo) {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo2"
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
// CHECK-NEXT:             "name": "bar",
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "GenericTypeAnnotation",
// CHECK-NEXT:                 "id": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "Foo"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "typeParameters": null
// CHECK-NEXT:               }
// CHECK-NEXT:             }
// CHECK-NEXT:           },
// CHECK-NEXT:           "shorthand": false
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       }
// CHECK-NEXT:     },

component Foo3('foo' as bar) {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo3"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ComponentParameter",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "StringLiteral",
// CHECK-NEXT:             "value": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "local": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "bar"
// CHECK-NEXT:           },
// CHECK-NEXT:           "shorthand": false
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       }
// CHECK-NEXT:     },

component Foo4(foo as {bar}) {}
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
// CHECK-NEXT:             "type": "ObjectPattern",
// CHECK-NEXT:             "properties": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Property",
// CHECK-NEXT:                 "key": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "bar"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "value": {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "bar"
// CHECK-NEXT:                 },
// CHECK-NEXT:                 "kind": "init",
// CHECK-NEXT:                 "computed": false,
// CHECK-NEXT:                 "method": false,
// CHECK-NEXT:                 "shorthand": true
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "shorthand": false
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       }
// CHECK-NEXT:     },

component Foo5(foo as [bar]) {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ComponentDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "Foo5"
// CHECK-NEXT:       },
// CHECK-NEXT:       "params": [
// CHECK-NEXT:         {
// CHECK-NEXT:           "type": "ComponentParameter",
// CHECK-NEXT:           "name": {
// CHECK-NEXT:             "type": "Identifier",
// CHECK-NEXT:             "name": "foo"
// CHECK-NEXT:           },
// CHECK-NEXT:           "local": {
// CHECK-NEXT:             "type": "ArrayPattern",
// CHECK-NEXT:             "elements": [
// CHECK-NEXT:               {
// CHECK-NEXT:                 "type": "Identifier",
// CHECK-NEXT:                 "name": "bar"
// CHECK-NEXT:               }
// CHECK-NEXT:             ]
// CHECK-NEXT:           },
// CHECK-NEXT:           "shorthand": false
// CHECK-NEXT:         }
// CHECK-NEXT:       ],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       }
// CHECK-NEXT:     }

// CHECK-NEXT:   ]
// CHECK-NEXT: }
