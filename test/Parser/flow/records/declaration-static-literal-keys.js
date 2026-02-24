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

  static 42: number = 0,

// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "RecordDeclarationStaticProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 42,
// CHECK-NEXT:               "raw": "42"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "NumberTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "NumericLiteral",
// CHECK-NEXT:               "value": 0,
// CHECK-NEXT:               "raw": "0"
// CHECK-NEXT:             }
// CHECK-NEXT:           },

  static 'foo': string = '',

// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "RecordDeclarationStaticProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "StringLiteral",
// CHECK-NEXT:               "value": "foo"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "StringTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "StringLiteral",
// CHECK-NEXT:               "value": ""
// CHECK-NEXT:             }
// CHECK-NEXT:           },

  static 2n: bigint = 0n,

// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "RecordDeclarationStaticProperty",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "BigIntLiteral",
// CHECK-NEXT:               "bigint": "2"
// CHECK-NEXT:             },
// CHECK-NEXT:             "typeAnnotation": {
// CHECK-NEXT:               "type": "TypeAnnotation",
// CHECK-NEXT:               "typeAnnotation": {
// CHECK-NEXT:                 "type": "BigIntTypeAnnotation"
// CHECK-NEXT:               }
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "BigIntLiteral",
// CHECK-NEXT:               "bigint": "0"
// CHECK-NEXT:             }
// CHECK-NEXT:           }

}

// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
