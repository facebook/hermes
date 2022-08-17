/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast -pretty-json %s | %FileCheck %s --match-full-lines

try {} catch ([a,b]) {}
// CHECK: {
// CHECK-NEXT:   "type": "Program",
// CHECK-NEXT:   "body": [
// CHECK-NEXT:     {
// CHECK-NEXT:       "type": "TryStatement",
// CHECK-NEXT:       "block": {
// CHECK-NEXT:         "type": "BlockStatement",
// CHECK-NEXT:         "body": []
// CHECK-NEXT:       },
// CHECK-NEXT:       "handler": {
// CHECK-NEXT:         "type": "CatchClause",
// CHECK-NEXT:         "param": {
// CHECK-NEXT:           "type": "ArrayPattern",
// CHECK-NEXT:           "elements": [
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "a"
// CHECK-NEXT:             },
// CHECK-NEXT:             {
// CHECK-NEXT:               "type": "Identifier",
// CHECK-NEXT:               "name": "b"
// CHECK-NEXT:             }
// CHECK-NEXT:           ]
// CHECK-NEXT:         },
// CHECK-NEXT:         "body": {
// CHECK-NEXT:           "type": "BlockStatement",
// CHECK-NEXT:           "body": []
// CHECK-NEXT:         }
// CHECK-NEXT:       },
// CHECK-NEXT:       "finalizer": null
// CHECK-NEXT:     }
// CHECK-NEXT:   ]
// CHECK-NEXT: }
