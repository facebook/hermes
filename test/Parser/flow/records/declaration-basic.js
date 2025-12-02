/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -parse-flow -Xparse-flow-records -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s

// CHECK-LABEL: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [

record R {

// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "RecordDeclaration",
// CHECK-NEXT:       "id": {
// CHECK-NEXT:         "type": "Identifier",
// CHECK-NEXT:         "name": "R"
// CHECK-NEXT:       },
// CHECK-NEXT:       "typeParameters": null,
// CHECK-NEXT:       "implements": [],
// CHECK-NEXT:       "body": {
// CHECK-NEXT:         "type": "RecordDeclarationBody",
// CHECK-NEXT:         "elements": [

  foo: string,

// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "RecordDeclarationProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "StringTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "defaultValue": null
// CHECK-NEXT:           },

  bar: boolean = false,

// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "RecordDeclarationProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "bar"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "BooleanTypeAnnotation"
// CHECK-NEXT:             },
// CHECK-NEXT:             "defaultValue": {
// CHECK-NEXT:               "type": "BooleanLiteral",
// CHECK-NEXT:               "value": false
// CHECK-NEXT:             }
// CHECK-NEXT:           }

}

// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
