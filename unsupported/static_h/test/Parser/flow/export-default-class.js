/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -commonjs -parse-flow -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

// CHECK-LABEL:   "body": {
// CHECK-NEXT:     "type": "BlockStatement",
// CHECK-NEXT:     "body": [

export default class<T> {}
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "ExportDefaultDeclaration",
// CHECK-NEXT:       "declaration": {
// CHECK-NEXT:         "type": "ClassDeclaration",
// CHECK-NEXT:         "id": null,
// CHECK-NEXT:         "typeParameters": {
// CHECK-NEXT:           "type": "TypeParameterDeclaration",
// CHECK-NEXT:           "params": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "TypeParameter",
// CHECK-NEXT:               "name": "T",
// CHECK-NEXT:               "bound": null,
// CHECK-NEXT:               "variance": null,
// CHECK-NEXT:               "default": null
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "superClass": null,
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "ClassBody",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     }
