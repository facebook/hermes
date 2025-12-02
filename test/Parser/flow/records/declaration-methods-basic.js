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

  equals(other: string) {
    return this.foo === other;
  }

// CHECK-NEXT:           {
// CHECK-NEXT:             "type": "MethodDefinition",
// CHECK-NEXT:             "key": {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "equals"
// CHECK-NEXT:             },
// CHECK-NEXT:             "value": {
// CHECK-NEXT:               "type": "FunctionExpression",
// CHECK-NEXT:               "id": null,
// CHECK-NEXT:               "params": [
// CHECK-NEXT:                 {
// CHECK-NEXT:                   "type": "Identifier",
// CHECK-NEXT:                   "name": "other",
// CHECK-NEXT:                   "typeAnnotation": {
// CHECK-NEXT:                     "type": "TypeAnnotation",
// CHECK-NEXT:                     "typeAnnotation": {
// CHECK-NEXT:                       "type": "StringTypeAnnotation"
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 }
// CHECK-NEXT:               ],
// CHECK-NEXT:               "body": {
// CHECK-NEXT:                 "type": "BlockStatement",
// CHECK-NEXT:                 "body": [
// CHECK-NEXT:                   {
// CHECK-NEXT:                     "type": "ReturnStatement",
// CHECK-NEXT:                     "argument": {
// CHECK-NEXT:                       "type": "BinaryExpression",
// CHECK-NEXT:                       "left": {
// CHECK-NEXT:                         "type": "MemberExpression",
// CHECK-NEXT:                         "object": {
// CHECK-NEXT:                           "type": "ThisExpression"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "property": {
// CHECK-NEXT:                           "type": "Identifier",
// CHECK-NEXT:                           "name": "foo"
// CHECK-NEXT:                         },
// CHECK-NEXT:                         "computed": false
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "right": {
// CHECK-NEXT:                         "type": "Identifier",
// CHECK-NEXT:                         "name": "other"
// CHECK-NEXT:                       },
// CHECK-NEXT:                       "operator": "==="
// CHECK-NEXT:                     }
// CHECK-NEXT:                   }
// CHECK-NEXT:                 ]
// CHECK-NEXT:               },
// CHECK-NEXT:               "generator": false,
// CHECK-NEXT:               "async": false
// CHECK-NEXT:             },
// CHECK-NEXT:             "kind": "method",
// CHECK-NEXT:             "computed": false,
// CHECK-NEXT:             "static": false
// CHECK-NEXT:           }

}

// CHECK-NEXT:         ]
// CHECK-NEXT:       }
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
