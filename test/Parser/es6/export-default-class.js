/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -commonjs -dump-ast -pretty-json %s | %FileCheck --match-full-lines %s

export default class {}
// CHECK-LABEL:     "body": [
// CHECK-NEXT:       {
// CHECK-NEXT:         "type": "ExportDefaultDeclaration",
// CHECK-NEXT:         "declaration": {
// CHECK-NEXT:           "type": "ClassDeclaration",
// CHECK-NEXT:           "id": null,
// CHECK-NEXT:           "superClass": null,
// CHECK-NEXT:           "body": {
// CHECK-NEXT:             "type": "ClassBody",
// CHECK-NEXT:             "body": []
// CHECK-NEXT:           }
// CHECK-NEXT:         }
// CHECK-NEXT:       }
// CHECK-NEXT:     ]
